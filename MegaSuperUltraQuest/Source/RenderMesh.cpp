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
	this->surfaceProperties.shininessExponent = 50.0;
}

/*virtual*/ RenderMeshInstance::~RenderMeshInstance()
{
}

void RenderMeshInstance::Render(Camera* camera, RenderPass renderPass)
{
	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	Shader* shader = nullptr;
	
	switch (renderPass)
	{
	case RenderPass::MAIN_PASS:
		shader = this->mesh->GetShader();
		break;
	case RenderPass::SHADOW_PASS:
		shader = this->mesh->GetShadowShader();
		break;
	}

	if (!shader)
		return;

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
		camera->GetProjectionMatrix(cameraToProjMat);

		Matrix4x4 objectToProjMat = cameraToProjMat * worldToCameraMat * objectToWorldMat;

		if (shader->GetConstantInfo("objectToProjection", constant))
			StoreShaderConstant(&mappedSubresource, constant, &objectToProjMat);

		if (shader->GetConstantInfo("objectToWorld", constant))
			StoreShaderConstant(&mappedSubresource, constant, &objectToWorldMat);

		if (shader->GetConstantInfo("lightDirection", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().lightDirection);

		if (shader->GetConstantInfo("directionalLightIntensity", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().directionalLightIntensity);

		if (shader->GetConstantInfo("ambientLightIntensity", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().ambientLightIntensity);

		if (shader->GetConstantInfo("shininessExponent", constant))
			StoreShaderConstant(&mappedSubresource, constant, &this->surfaceProperties.shininessExponent);

		if (shader->GetConstantInfo("lightColor", constant))
			StoreShaderConstant(&mappedSubresource, constant, &Game::Get()->GetLightParams().lightColor);

		if (shader->GetConstantInfo("cameraEyePoint", constant))
			StoreShaderConstant(&mappedSubresource, constant, &camera->GetCameraToWorldTransform().translation);

		if (renderPass == RenderPass::MAIN_PASS)
		{
			Camera* lightSourceCamera = Game::Get()->GetLightSourceCamera();
			if (lightSourceCamera)
			{
				const Transform& lightCameraToWorld = lightSourceCamera->GetCameraToWorldTransform();
				const Camera::OrthographicParams& orthoParams = lightSourceCamera->GetOrthographicParameters();

				Vector3 lightCameraEyePoint = lightCameraToWorld.translation;
				Vector3 lightCameraXAxis, lightCameraYAxis, lightCameraZAxis;
				double lightCameraWidth = 0.0, lightCameraHeight = 0.0;
				double lightCameraNear = 0.0, lightCameraFar = 0.0;

				lightCameraToWorld.matrix.GetColumnVectors(lightCameraXAxis, lightCameraYAxis, lightCameraZAxis);
				lightCameraWidth = orthoParams.width;
				lightCameraHeight = orthoParams.height;
				lightCameraNear = orthoParams.nearClip;
				lightCameraFar = orthoParams.farClip;

				if (shader->GetConstantInfo("lightCameraEyePoint", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraEyePoint);

				if (shader->GetConstantInfo("lightCameraXAxis", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraXAxis);

				if (shader->GetConstantInfo("lightCameraYAxis", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraYAxis);

				if (shader->GetConstantInfo("lightCameraWidth", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraWidth);

				if (shader->GetConstantInfo("lightCameraHeight", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraHeight);

				if (shader->GetConstantInfo("lightCameraNear", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraNear);

				if (shader->GetConstantInfo("lightCameraFar", constant))
					StoreShaderConstant(&mappedSubresource, constant, &lightCameraFar);
			}
		}

		deviceContext->Unmap(constantsBuffer, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &constantsBuffer);
		deviceContext->PSSetConstantBuffers(0, 1, &constantsBuffer);
	}

	if (renderPass == RenderPass::MAIN_PASS)
	{
		std::vector<ID3D11ShaderResourceView*> shaderResourceViewArray;
		std::vector<ID3D11SamplerState*> samplerStateArray;

		if (texture)
		{
			shaderResourceViewArray.push_back(texture->GetTextureView());
			samplerStateArray.push_back(texture->GetSamplerState());
		}

		ID3D11ShaderResourceView* shadowBufferResourceView = Game::Get()->GetShadowBufferResourceViewForShader();
		ID3D11SamplerState* shadowBufferSamplerState = Game::Get()->GetShadowBufferSamplerState();
		
		if (shadowBufferResourceView && shadowBufferSamplerState)
		{
			shaderResourceViewArray.push_back(shadowBufferResourceView);
			samplerStateArray.push_back(shadowBufferSamplerState);
		}

		deviceContext->PSSetShaderResources(0, shaderResourceViewArray.size(), shaderResourceViewArray.data());
		deviceContext->PSSetSamplers(0, samplerStateArray.size(), samplerStateArray.data());
	}
	else if (renderPass == RenderPass::SHADOW_PASS)
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

	this->mainPassShader.SafeSet(asset.Get());

	if (jsonDoc.HasMember("shadow_shader"))
	{
		std::string shadowShaderFile = jsonDoc["shadow_shader"].GetString();
		if (!assetCache->GrabAsset(shadowShaderFile, asset))
			return false;

		this->shadowPassShader.SafeSet(asset.Get());
	}

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