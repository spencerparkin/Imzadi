#include "TextRenderObject.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Game.h"
#include "Log.h"
#include "Camera.h"

using namespace Imzadi;

//---------------------------- TextRenderObject ----------------------------

TextRenderObject::TextRenderObject()
{
	this->flags = 0;
	this->vertexBuffer = nullptr;
	this->numElements = 0;
	this->maxCharsPerLine = 32;
	this->objectToTargetSpace.SetIdentity();
	this->foreColor.SetComponents(1.0, 1.0, 1.0);
	this->backColor.SetComponents(0.0, 0.0, 0.0);
	this->backAlpha = 1.0;
}

/*virtual*/ TextRenderObject::~TextRenderObject()
{
	this->font.Reset();
	SafeRelease(this->vertexBuffer);
}

void TextRenderObject::SetMaxCharsPerLine(uint32_t maxChars)
{
	this->maxCharsPerLine = maxChars;
}

uint32_t TextRenderObject::GetMaxCharsPerLine() const
{
	return this->maxCharsPerLine;
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

	std::vector<std::string> lineArray;
	if ((this->flags & Flag::MULTI_LINE) == 0)
		lineArray.push_back(this->text);
	else if ((this->flags & Flag::USE_NEWLINE_CHARS) != 0)
		this->SplitString(this->text, lineArray, "\n");
	else
	{
		// Get an array of words from the text.
		std::vector<std::string> wordArray;
		this->SplitString(this->text, wordArray);

		// Build each line, making sure not to exceed the maximum for each line.
		uint32_t i = 0, j = 0;
		std::string line;
		for (uint32_t i = 0; i < wordArray.size(); i++)
		{
			const std::string& word = wordArray[i];
			if (j + word.length() <= this->maxCharsPerLine)
			{
				line += (line.length() > 0 ? " " : "") + word;
				j += word.length();
			}
			else
			{
				if (line.size() == 0)
					return;
				lineArray.push_back(line);
				line = word;
				j = word.length();
			}
		}

		if (line.length() > 0)
			lineArray.push_back(line);
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	result = deviceContext->Map(this->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	if (FAILED(result))
		return;

	this->numElements = 0;
	auto floatPtr = static_cast<float*>(mappedSubresource.pData);

	if ((this->flags & Flag::DRAW_BACKGROUND) != 0)
	{
		// Reserve space for an initial quad that will serve as the background.
		Font::CharacterInfo info;
		if (this->font->GetCharInfo(' ', info))
		{
			*floatPtr++ = 0.0f;
			*floatPtr++ = 0.0f;
			*floatPtr++ = info.minUV.x;
			*floatPtr++ = info.maxUV.y;

			*floatPtr++ = 0.0f;
			*floatPtr++ = 0.0f;
			*floatPtr++ = info.maxUV.x;
			*floatPtr++ = info.maxUV.y;

			*floatPtr++ = 0.0f;
			*floatPtr++ = 0.0f;
			*floatPtr++ = info.maxUV.x;
			*floatPtr++ = info.minUV.y;

			*floatPtr++ = 0.0f;
			*floatPtr++ = 0.0f;
			*floatPtr++ = info.minUV.x;
			*floatPtr++ = info.minUV.y;

			this->numElements += 6;	// 1 quad per character, 2 triangles per quad, 3 vertices per triangle.
		}
	}

	// Calculate bounding boxes for each line.
	std::vector<AxisAlignedBoundingBox> lineBoxArray;
	std::vector<Vector2> penStartLocationArray;
	for (uint32_t i = 0; i < (uint32_t)lineArray.size(); i++)
	{
		lineBoxArray.push_back(this->CalculateStringBox(lineArray[i]));
		penStartLocationArray.push_back(Vector2(0.0, 0.0));
	}

	// Calculate the largest box height.
	double maxBoxHeight = 0.0;
	for (uint32_t i = 0; i < (uint32_t)lineBoxArray.size(); i++)
	{
		AxisAlignedBoundingBox& lineBox = lineBoxArray[i];
		double boxWidth = 0.0, boxHeight = 0.0, boxDepth = 0.0;
		lineBox.GetDimensions(boxWidth, boxHeight, boxDepth);
		maxBoxHeight = IMZADI_MAX(maxBoxHeight, boxHeight);
	}

	// Lay the boxes on top of one another.
	for (uint32_t i = 0; i < (uint32_t)lineBoxArray.size(); i++)
	{
		uint32_t j = lineBoxArray.size() - 1 - i;
		AxisAlignedBoundingBox& lineBox = lineBoxArray[j];
		Vector2& penStartLocation = penStartLocationArray[j];
		lineBox.minCorner.y += i * maxBoxHeight;
		lineBox.maxCorner.y += i * maxBoxHeight;
		penStartLocation.y += i * maxBoxHeight;
	}

	// Align/justify the boxes left/center/right.
	for (uint32_t i = 0; i < (uint32_t)lineBoxArray.size(); i++)
	{
		AxisAlignedBoundingBox& lineBox = lineBoxArray[i];
		Vector2& penStartLocation = penStartLocationArray[i];

		double boxWidth = 0.0, boxHeight = 0.0, boxDepth = 0.0;
		lineBox.GetDimensions(boxWidth, boxHeight, boxDepth);

		if ((this->flags & Flag::RIGHT_JUSTIFY) != 0)
		{
			lineBox.minCorner.x -= boxWidth;
			lineBox.maxCorner.x -= boxWidth;
			penStartLocation.x -= boxWidth;
		}
		else if ((this->flags & Flag::CENTER_JUSTIFY) != 0)
		{
			lineBox.minCorner.x -= boxWidth / 2.0;
			lineBox.maxCorner.x -= boxWidth / 2.0;
			penStartLocation.x -= boxWidth / 2.0;
		}
	}

	// Finally, render the lines of text.
	for (uint32_t i = 0; i < (uint32_t)lineBoxArray.size(); i++)
	{
		const std::string& line = lineArray[i];
		Vector2 penLocation = penStartLocationArray[i];

		for (uint32_t j = 0; line.c_str()[j] != '\0'; j++)
		{
			char ch = line.c_str()[j];
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

				this->numElements += 6;	// 1 quad per character, 2 triangles per quad, 3 vertices per triangle.
			}

			penLocation.x += info.advance;
		}
	}

	if ((this->flags & Flag::DRAW_BACKGROUND) != 0)
	{
		floatPtr = static_cast<float*>(mappedSubresource.pData);

		AxisAlignedBoundingBox backgroundBox;
		backgroundBox.MakeReadyForExpansion();
		for (uint32_t i = 0; i < (uint32_t)lineBoxArray.size(); i++)
			backgroundBox.Expand(lineBoxArray[i]);

		static float margin = 0.005;
		backgroundBox.minCorner.x -= margin;
		backgroundBox.maxCorner.x += margin;
		backgroundBox.minCorner.y -= margin;
		backgroundBox.maxCorner.y += margin;

		floatPtr[0] = backgroundBox.minCorner.x;
		floatPtr[1] = backgroundBox.minCorner.y;
		floatPtr[4] = backgroundBox.maxCorner.x;
		floatPtr[5] = backgroundBox.minCorner.y;
		floatPtr[8] = backgroundBox.maxCorner.x;
		floatPtr[9] = backgroundBox.maxCorner.y;
		floatPtr[12] = backgroundBox.minCorner.x;
		floatPtr[13] = backgroundBox.maxCorner.y;
	}

	deviceContext->Unmap(this->vertexBuffer, 0);
}

/*static*/ void TextRenderObject::SplitString(const std::string& givenString, std::vector<std::string>& tokenArray, const std::string& delimiter /*= " "*/)
{
	char* buffer = new char[givenString.length() + 1];
	::strcpy_s(buffer, givenString.length() + 1, givenString.c_str());
	char* context = nullptr;
	char* tokenBuffer = ::strtok_s(buffer, delimiter.c_str(), &context);
	while (tokenBuffer)
	{
		std::string token(tokenBuffer);
		tokenArray.push_back(token);
		tokenBuffer = ::strtok_s(nullptr, delimiter.c_str(), &context);
	}
	delete[] buffer;
}

AxisAlignedBoundingBox TextRenderObject::CalculateStringBox(const std::string& givenString)
{
	AxisAlignedBoundingBox stringBox;
	stringBox.MakeReadyForExpansion();

	if (givenString.length() == 0)
		stringBox.Expand(Vector3(0.0, 0.0, 0.0));

	Vector2 penLocation(0.0, 0.0);

	for (int i = 0; givenString.c_str()[i] != '\0'; i++)
	{
		char ch = givenString.c_str()[i];
		Font::CharacterInfo info;
		if (!this->font->GetCharInfo(ch, info))
			continue;

		if (info.minUV != info.maxUV)
		{
			Vector2 glyphLocation = penLocation + info.penOffset;

			stringBox.Expand(Vector3(glyphLocation.x, glyphLocation.y, 0.0));
			stringBox.Expand(Vector3(glyphLocation.x + info.width, glyphLocation.y, 0.0));
			stringBox.Expand(Vector3(glyphLocation.x + info.width, glyphLocation.y + info.height, 0.0));
			stringBox.Expand(Vector3(glyphLocation.x, glyphLocation.y + info.height, 0.0));
		}

		penLocation.x += info.advance;
	}

	return stringBox;
}

/*virtual*/ void TextRenderObject::Render(Camera* camera, RenderPass renderPass)
{
	if (renderPass != RenderPass::MAIN_PASS)
		return;

	if (this->text.length() == 0)
		return;

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Game::Get()->GetBlendStateCache()->SetState(&blendDesc);

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	Game::Get()->GetRasterStateCache()->SetState(&rasterizerDesc);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = ((this->flags & Flag::DRAW_BACKGROUND) != 0) ? FALSE : TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0;
	depthStencilDesc.StencilWriteMask = 0;
	Game::Get()->GetDepthStencilStateCache()->SetState(&depthStencilDesc);

	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	Matrix4x4 objectToProjMat;

	if ((this->flags & Flag::STICK_WITH_CAMERA_PROJ) != 0)
		this->objectToTargetSpace.GetToMatrix(objectToProjMat);
	else
	{
		Matrix4x4 cameraToProjMat;
		camera->GetProjectionMatrix(cameraToProjMat);

		if ((this->flags & Flag::STICK_WITH_CAMERA) != 0)
		{
			Matrix4x4 objectToCameraMat;
			this->objectToTargetSpace.GetToMatrix(objectToCameraMat);

			objectToProjMat = cameraToProjMat * objectToCameraMat;
		}
		else
		{
			Matrix4x4 objectToWorldMat, worldToCameraMat;
			this->objectToTargetSpace.GetToMatrix(objectToWorldMat);
			camera->GetWorldToCameraTransform().GetToMatrix(worldToCameraMat);

			objectToProjMat = cameraToProjMat * worldToCameraMat * objectToWorldMat;

			if ((this->flags & Flag::ALWAYS_FACING_CAMERA) != 0)
			{
				Matrix4x4 rotationMat(camera->GetCameraToWorldTransform().matrix);
				objectToProjMat = objectToProjMat * rotationMat;
			}

			if ((this->flags & Flag::CONSTANT_SIZE) != 0)
			{
				double scale = (camera->GetEyePoint() - this->objectToTargetSpace.translation).Length();
				Matrix4x4 scaleMat;
				scaleMat.SetScale(Vector3(scale, scale, scale));
				objectToProjMat = objectToProjMat * scaleMat;
			}
		}
	}

	bool legible = (this->flags & Flag::DRAW_BACKGROUND) != 0;
	Shader* shader = this->font->GetShader(legible);

	ID3D11Buffer* constantsBuffer = shader->GetConstantsBuffer();
	if (!constantsBuffer)
		return;

	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	HRESULT result = deviceContext->Map(constantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	if (FAILED(result))
		return;

	const Shader::Constant* constant = nullptr;

	if (shader->GetConstantInfo("objectToProjection", constant))
		StoreShaderConstant(&mappedSubresource, constant, &objectToProjMat);

	if ((this->flags & Flag::DRAW_BACKGROUND) != 0)
	{
		if (shader->GetConstantInfo("textForeColor", constant))
			StoreShaderConstant(&mappedSubresource, constant, &this->foreColor);

		if (shader->GetConstantInfo("textBackColor", constant))
			StoreShaderConstant(&mappedSubresource, constant, &this->backColor);

		if (shader->GetConstantInfo("textBackAlpha", constant))
			StoreShaderConstant(&mappedSubresource, constant, &this->backAlpha);
	}
	else
	{
		if (shader->GetConstantInfo("textColor", constant))
			StoreShaderConstant(&mappedSubresource, constant, &this->foreColor);
	}

	double zFactor = ((this->flags & Flag::ALWAYS_ON_TOP) != 0) ? 0.0 : 1.0;
	if (shader->GetConstantInfo("zFactor", constant))
		StoreShaderConstant(&mappedSubresource, constant, &zFactor);

	deviceContext->Unmap(constantsBuffer, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(shader->GetInputLayout());

	deviceContext->VSSetShader(shader->GetVertexShader(), NULL, 0);
	deviceContext->PSSetShader(shader->GetPixelShader(), NULL, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &constantsBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &constantsBuffer);

	Texture* textureAtlas = this->font->GetTextureAtlas();
	ID3D11ShaderResourceView* textureAtlasView = textureAtlas->GetTextureView();
	ID3D11SamplerState* textureAtlasSamplerState = Game::Get()->GetGeneralSamplerState();
	deviceContext->PSSetShaderResources(0, 1, &textureAtlasView);
	deviceContext->PSSetSamplers(0, 1, &textureAtlasSamplerState);

	UINT stride = 4 * sizeof(float);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);

	ID3D11Buffer* indexBuffer = font->GetIndexBuffer();
	DXGI_FORMAT format = DXGI_FORMAT_R16_UINT;
	deviceContext->IASetIndexBuffer(indexBuffer, format, 0);

	deviceContext->DrawIndexed(this->numElements, 0, 0);
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
	if (this->flags != givenFlags)
	{
		this->flags = givenFlags;
		this->flags |= Flag::REBUILD_VERTEX_BUFFER;
	}
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

void TextRenderObject::SetForegroundColor(const Vector3& color)
{
	this->foreColor = color;
}

const Vector3& TextRenderObject::GetForegroundColor() const
{
	return this->foreColor;
}

void TextRenderObject::SetBackgroundColor(const Vector3& color)
{
	this->backColor = color;
}

const Vector3& TextRenderObject::GetBackgroundColor() const
{
	return this->backColor;
}

void TextRenderObject::SetBackgroundAlpha(double alpha)
{
	this->backAlpha = alpha;
}

double TextRenderObject::GetBackgroundAlpha() const
{
	return this->backAlpha;
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

Font* TextRenderObject::GetFont()
{
	return this->font.Get();
}

void TextRenderObject::SetTransform(const Transform& transform)
{
	this->objectToTargetSpace = transform;
}

const Transform& TextRenderObject::GetTransform() const
{
	return this->objectToTargetSpace;
}

//---------------------------- FPSRenderObject ----------------------------

FPSRenderObject::FPSRenderObject()
{
	this->deltaTimeListMaxSize = 16;
}

/*virtual*/ FPSRenderObject::~FPSRenderObject()
{
}

/*virtual*/ void FPSRenderObject::Prepare()
{
	this->deltaTimeList.push_back(Game::Get()->GetDeltaTime());
	while (this->deltaTimeList.size() > this->deltaTimeListMaxSize)
		this->deltaTimeList.pop_front();

	double averageFrameTimeSeconds = 0.0;
	for (double deltaTimeSeconds : this->deltaTimeList)
		averageFrameTimeSeconds += deltaTimeSeconds;
	averageFrameTimeSeconds /= double(this->deltaTimeList.size());

	double frameRateFPS = 1.0 / averageFrameTimeSeconds;
	this->SetText(std::format("FPS: {:.2f}", frameRateFPS));

	double aspectRatio = Game::Get()->GetAspectRatio();

	Transform scale;
	scale.SetIdentity();
	scale.matrix.SetNonUniformScale(Vector3(1.0, aspectRatio, 1.0));

	Transform translate;
	translate.SetIdentity();
	translate.translation.SetComponents(0.7, 0.9, -0.5);

	this->SetTransform(translate * scale);

	this->SetFlags(Flag::ALWAYS_ON_TOP | Flag::RIGHT_JUSTIFY | Flag::STICK_WITH_CAMERA_PROJ);

	TextRenderObject::Prepare();
}