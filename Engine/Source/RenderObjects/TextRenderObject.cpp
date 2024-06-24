#include "TextRenderObject.h"
#include "Game.h"
#include "Log.h"
#include "Camera.h"

using namespace Imzadi;

TextRenderObject::TextRenderObject()
{
	this->flags = 0;
	this->vertexBuffer = nullptr;
	this->objectToTargetSpace.SetIdentity();
}

/*virtual*/ TextRenderObject::~TextRenderObject()
{
	this->font.Reset();
	SafeRelease(this->vertexBuffer);
}

/*virtual*/ void TextRenderObject::Prepare()
{
	if ((this->flags & Flag::REBUILD_VERTEX_BUFFER) == 0)
		return;

	this->flags &= ~Flag::REBUILD_VERTEX_BUFFER;

	HRESULT result = 0;

	if (!this->vertexBuffer)
	{
		D3D11_BUFFER_DESC indexBufferDesc{};
		this->font->GetIndexBuffer()->GetDesc(&indexBufferDesc);

		uint32_t maxCharacters = indexBufferDesc.ByteWidth / (sizeof(uint16_t) * 6);

		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.ByteWidth = sizeof(float) * 16 * maxCharacters;	// 1 quad per character, 4 vertices per quad, 4 floats per vertex.
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		result = Game::Get()->GetDevice()->CreateBuffer(&vertexBufferDesc, NULL, &this->vertexBuffer);
		if (FAILED(result))
			IMZADI_LOG_FATAL_ERROR("Failed to allocate text vertex buffer of size %d bytes.  Error code: %d", vertexBufferDesc.ByteWidth, result);

		if (!this->vertexBuffer)
			return;
	}

	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	result = deviceContext->Map(this->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	if (FAILED(result))
		return;

	Vector2 penLocation(0.0, 0.0);

	// TODO: I know this is very wrong, but just do this for now to get something working.
	auto floatPtr = static_cast<float*>(mappedSubresource.pData);
	for (int i = 0; this->text.c_str()[i] != '\0'; i++)
	{
		char ch = this->text.c_str()[i];
		Font::CharacterInfo info;
		if (!this->font->GetCharInfo(ch, info))
			continue;

		if (info.minUV != info.maxUV)
		{
			Vector2 glyphLocation = penLocation + info.penOffset;

			*floatPtr++ = glyphLocation.x;
			*floatPtr++ = glyphLocation.y;
			*floatPtr++ = info.minUV.x;
			*floatPtr++ = info.maxUV.y;

			*floatPtr++ = glyphLocation.x + info.width;
			*floatPtr++ = glyphLocation.y;
			*floatPtr++ = info.maxUV.x;
			*floatPtr++ = info.maxUV.y;

			*floatPtr++ = glyphLocation.x + info.width;
			*floatPtr++ = glyphLocation.y + info.height;
			*floatPtr++ = info.maxUV.x;
			*floatPtr++ = info.minUV.y;

			*floatPtr++ = glyphLocation.x;
			*floatPtr++ = glyphLocation.y + info.height;
			*floatPtr++ = info.minUV.x;
			*floatPtr++ = info.minUV.y;
		}

		penLocation.x += info.advance;
	}

	deviceContext->Unmap(this->vertexBuffer, 0);
}

/*virtual*/ void TextRenderObject::Render(Camera* camera, RenderPass renderPass)
{
	if (renderPass != RenderPass::MAIN_PASS)
		return;

	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	Shader* shader = this->font->GetShader();

	ID3D11Buffer* constantsBuffer = shader->GetConstantsBuffer();
	if (!constantsBuffer)
		return;

	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	HRESULT result = deviceContext->Map(constantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	if (FAILED(result))
		return;

	Matrix4x4 worldToCameraMat;
	camera->GetWorldToCameraTransform().GetToMatrix(worldToCameraMat);

	Matrix4x4 cameraToProjMat;
	camera->GetProjectionMatrix(cameraToProjMat);

	Matrix4x4 objectToWorldMat;
	this->objectToTargetSpace.GetToMatrix(objectToWorldMat);

	Matrix4x4 objectToProjMat;
	objectToProjMat = cameraToProjMat * worldToCameraMat * objectToWorldMat;

	const Shader::Constant* constant = nullptr;

	if (shader->GetConstantInfo("objectToProjection", constant))
		StoreShaderConstant(&mappedSubresource, constant, &objectToProjMat);

	if (shader->GetConstantInfo("textColor", constant))
		StoreShaderConstant(&mappedSubresource, constant, &this->color);

	deviceContext->Unmap(constantsBuffer, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(shader->GetInputLayout());

	deviceContext->VSSetShader(shader->GetVertexShader(), NULL, 0);
	deviceContext->PSSetShader(shader->GetPixelShader(), NULL, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &constantsBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &constantsBuffer);

	Texture* textureAtlas = this->font->GetTextureAtlas();
	ID3D11ShaderResourceView* textureAtlasView = textureAtlas->GetTextureView();
	ID3D11SamplerState* textureAtlasSamplerState = textureAtlas->GetSamplerState();
	deviceContext->PSSetShaderResources(0, 1, &textureAtlasView);
	deviceContext->PSSetSamplers(0, 1, &textureAtlasSamplerState);

	UINT stride = 4 * sizeof(float);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);

	ID3D11Buffer* indexBuffer = font->GetIndexBuffer();
	DXGI_FORMAT format = DXGI_FORMAT_R16_UINT;
	deviceContext->IASetIndexBuffer(indexBuffer, format, 0);

	UINT numElements = this->text.length() * 6;
	deviceContext->DrawIndexed(numElements, 0, 0);
}

/*virtual*/ void TextRenderObject::GetWorldBoundingSphere(Imzadi::Vector3& center, double& radius) const
{
	// TODO: Write this.
}

/*virtual*/ int TextRenderObject::SortKey() const
{
	return 1;
}

void TextRenderObject::SetFlags(uint32_t givenFlags)
{
	this->flags = givenFlags;
}

uint32_t TextRenderObject::GetFlags() const
{
	return this->flags;
}

void TextRenderObject::SetText(const std::string& givenText)
{
	if (this->text != givenText)
	{
		this->text = givenText;
		this->flags |= Flag::REBUILD_VERTEX_BUFFER;
	}
}

const std::string& TextRenderObject::GetText() const
{
	return this->text;
}

void TextRenderObject::SetColor(const Vector3& color)
{
	this->color = color;
}

const Vector3& TextRenderObject::GetColor() const
{
	return this->color;
}

bool TextRenderObject::SetFont(const std::string& fontName)
{
	std::string fontFile = std::format("Fonts/{}.font", fontName.c_str());
	Reference<Asset> asset;
	if (!Game::Get()->GetAssetCache()->LoadAsset(fontFile, asset))
	{
		IMZADI_LOG_ERROR("No font with name \"%s\" found.", fontName.c_str());
		return false;
	}

	this->font.SafeSet(asset.Get());
	if (!this->font)
	{
		IMZADI_LOG_ERROR("Whatever loaded from \"%s\" was not a font.", fontFile.c_str());
		return false;
	}

	return true;
}

void TextRenderObject::SetTransform(const Transform& transform)
{
	this->objectToTargetSpace = transform;
}

const Transform& TextRenderObject::GetTransform() const
{
	return this->objectToTargetSpace;
}