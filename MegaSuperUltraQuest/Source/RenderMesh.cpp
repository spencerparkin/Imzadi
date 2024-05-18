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
}

/*virtual*/ RenderMeshInstance::~RenderMeshInstance()
{
}

void RenderMeshInstance::Render(Scene* scene)
{
	Camera* camera = scene->GetCamera();
	if (!camera)
		return;

	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	Shader* shader = this->mesh->GetShader();
	Buffer* vertexBuffer = this->mesh->GetVertexBuffer();
	Buffer* indexBuffer = this->mesh->GetIndexBuffer();

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

		// Is there somewhere in the constants buffer where we can communicate the object-space to project-space tranformation matrix?
		if (shader->GetConstantInfo("object_to_projection", constant) && constant->size == 16 * sizeof(float) && constant->format == DXGI_FORMAT_R32_FLOAT)
		{
			Matrix4x4 worldToCameraMat;
			camera->GetWorldToCameraTransform().GetToMatrix(worldToCameraMat);

			Matrix4x4 objectToWorldMat;
			this->objectToWorld.GetToMatrix(objectToWorldMat);

			Matrix4x4 cameraToProjMat;
			camera->GetFrustum().GetToProjectionMatrix(cameraToProjMat);

			Matrix4x4 objectToProjMat = cameraToProjMat * worldToCameraMat * objectToWorldMat;

			float* ele = (float*)&((uint8_t*)mappedSubresource.pData)[constant->offset];
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					*ele++ = float(objectToProjMat.ele[j][i]);	// Note the swap of i and j here!
		}

		// Is there somewhere in the constants buffer where we can communicate a color to the shader?
		if (shader->GetConstantInfo("color", constant) && constant->size == 4 * sizeof(float) && constant->format == DXGI_FORMAT_R32_FLOAT)
		{
			Vector4 color(0.0, 1.0, 0.5, 1.0);	// Just use this for now.
			float* colorArray = (float*)&((uint8_t*)mappedSubresource.pData)[constant->offset];
			colorArray[0] = color.x;
			colorArray[1] = color.y;
			colorArray[2] = color.z;
			colorArray[3] = color.w;
		}

		deviceContext->Unmap(constantsBuffer, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &constantsBuffer);
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