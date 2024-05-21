#include "RenderMesh.h"
#include "Camera.h"
#include "Scene.h"
#include "Buffer.h"
#include "Game.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector4.h"

using namespace Collision;

//---------------------------------- RenderMeshInstance ----------------------------------

RenderMeshInstance::RenderMeshInstance()
{
	this->objectToWorld.SetIdentity();
	this->color.SetComponents(1.0, 0.0, 0.0, 1.0);
}

/*virtual*/ RenderMeshInstance::~RenderMeshInstance()
{
}

void RenderMeshInstance::Render(Camera* camera)
{
	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	Shader* shader = this->mesh->GetShader();
	Buffer* vertexBuffer = this->mesh->GetVertexBuffer();
	Buffer* indexBuffer = this->mesh->GetIndexBuffer();
	Texture* texture = this->mesh->GetTexture();

	deviceContext->IASetPrimitiveTopology(this->mesh->GetPrimType());
	deviceContext->IASetInputLayout(shader->GetInputLayout());

	deviceContext->VSSetShader(shader->GetVertexShader(), NULL, 0);
	deviceContext->PSSetShader(shader->GetPixelShader(), NULL, 0);

	UINT stride = vertexBuffer->GetStride();
	UINT offset = 0;
	ID3D11Buffer* vertexBufferIface = vertexBuffer->GetBuffer();
	deviceContext->IASetVertexBuffers(0, 1, &vertexBufferIface, &stride, &offset);

	ID3D11Buffer* constantsBuffer = shader->GetConstantsBuffer();
	if (constantsBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		deviceContext->Map(constantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);

		UINT bufferSize = shader->GetConstantsBufferSize();
		::memset(mappedSubresource.pData, 0, bufferSize);

		const Shader::Constant* constant = nullptr;

		Matrix4x4 worldToCameraMat;
		camera->GetWorldToCameraTransform().GetToMatrix(worldToCameraMat);

		Matrix4x4 objectToWorldMat;
		this->objectToWorld.GetToMatrix(objectToWorldMat);

		Matrix4x4 cameraToProjMat;
		camera->GetFrustum().GetToProjectionMatrix(cameraToProjMat);

		Matrix4x4 objectToProjMat = cameraToProjMat * worldToCameraMat * objectToWorldMat;

		if (shader->GetConstantInfo("object_to_projection", constant))
			StoreShaderConstant(&mappedSubresource, constant, &objectToProjMat);

		if (shader->GetConstantInfo("object_to_world", constant))
			StoreShaderConstant(&mappedSubresource, constant, &objectToWorldMat);

		if (shader->GetConstantInfo("color", constant))
			StoreShaderConstant(&mappedSubresource, constant, &this->color);

		if (shader->GetConstantInfo("light_direction", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().lightDirection);

		if (shader->GetConstantInfo("light_intensity", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().lightIntensity);

		if (shader->GetConstantInfo("light_color", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().lightColor);

		deviceContext->Unmap(constantsBuffer, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &constantsBuffer);
		deviceContext->PSSetConstantBuffers(0, 1, &constantsBuffer);
	}

	if (texture)
	{
		ID3D11ShaderResourceView* textureView = texture->GetTextureView();
		deviceContext->PSSetShaderResources(0, 1, &textureView);

		ID3D11SamplerState* samplerState = texture->GetSamplerState();
		deviceContext->PSSetSamplers(0, 1, &samplerState);
	}
	else
	{
		deviceContext->PSSetShaderResources(0, 0, NULL);
		deviceContext->PSSetSamplers(0, 0, NULL);
	}

	if (!indexBuffer)
		deviceContext->Draw(this->mesh->GetVertexBuffer()->GetNumElements(), 0);
	else
	{
		ID3D11Buffer* indexBufferIface = indexBuffer->GetBuffer();
		DXGI_FORMAT format = indexBuffer->GetFormat();
		deviceContext->IASetIndexBuffer(indexBufferIface, format, 0);

		UINT numElements = indexBuffer->GetNumElements();
		deviceContext->DrawIndexed(numElements, 0, 0);
	}
}

/*virtual*/ void RenderMeshInstance::GetWorldBoundingSphere(Collision::Vector3& center, double& radius) const
{
	this->objectSpaceBoundingBox.GetSphere(center, radius);
	center = this->objectToWorld.TransformPoint(center);
}

//---------------------------------- RenderMeshAsset ----------------------------------

RenderMeshAsset::RenderMeshAsset()
{
	this->primType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

/*virtual*/ RenderMeshAsset::~RenderMeshAsset()
{
}

/*virtual*/ bool RenderMeshAsset::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	Reference<Asset> asset;

	if (!jsonDoc.IsObject())
		return false;

	if (jsonDoc.HasMember("bounding_box"))
	{
		if (!assetCache->LoadBoundingBox(jsonDoc["bounding_box"], this->objectSpaceBoundingBox))
			return false;
	}

	if (!jsonDoc.HasMember("vertex_buffer"))
		return false;

	std::string vertexBufferFile = jsonDoc["vertex_buffer"].GetString();
	if (!assetCache->GrabAsset(vertexBufferFile, asset))
		return false;

	this->vertexBuffer.SafeSet(asset.Get());

	if (!jsonDoc.HasMember("shader"))
		return false;

	std::string shaderFile = jsonDoc["shader"].GetString();
	if (!assetCache->GrabAsset(shaderFile, asset))
		return false;

	this->shader.SafeSet(asset.Get());

	if (jsonDoc.HasMember("index_buffer"))
	{
		std::string indexBufferFile = jsonDoc["index_buffer"].GetString();
		
		if (!assetCache->GrabAsset(indexBufferFile, asset))
			return false;

		this->indexBuffer.SafeSet(asset.Get());
	}

	if (jsonDoc.HasMember("texture"))
	{
		std::string textureFile = jsonDoc["texture"].GetString();

		if (!assetCache->GrabAsset(textureFile, asset))
			return false;

		this->texture.SafeSet(asset.Get());
	}

	if (!jsonDoc.HasMember("primitive_type"))
		return false;
	
	std::string primTypeStr = jsonDoc["primitive_type"].GetString();
	if (primTypeStr == "TRIANGLE_LIST")
		this->primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	if (this->primType == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
		return false;

	return true;
}

/*virtual*/ bool RenderMeshAsset::Unload()
{
	// Note that we don't want to unload our texture, buffers,
	// or shaders here, because they may be in use by some
	// other render mesh.  We don't leak anything by doing
	// nothing here, because those types of assets unload themselves.
	return true;
}

/*virtual*/ bool RenderMeshAsset::MakeRenderInstance(Reference<RenderObject>& renderObject)
{
	renderObject.Set(new RenderMeshInstance());
	auto instance = dynamic_cast<RenderMeshInstance*>(renderObject.Get());
	instance->SetRenderMesh(this);
	instance->SetBoundingBox(this->objectSpaceBoundingBox);
	return true;
}