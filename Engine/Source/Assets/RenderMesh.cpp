#include "RenderMesh.h"
#include "Camera.h"
#include "Scene.h"
#include "Buffer.h"
#include "Game.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector4.h"
#include "RenderObjects/RenderMeshInstance.h"

using namespace Imzadi;

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
		if (!this->LoadBoundingBox(jsonDoc["bounding_box"], this->objectSpaceBoundingBox))
			return false;
	}

	if (!jsonDoc.HasMember("vertex_buffer"))
		return false;

	std::string vertexBufferFile = jsonDoc["vertex_buffer"].GetString();
	if (!assetCache->LoadAsset(vertexBufferFile, asset))
		return false;

	this->vertexBuffer.SafeSet(asset.Get());

	if (!jsonDoc.HasMember("shader"))
		return false;

	std::string shaderFile = jsonDoc["shader"].GetString();
	if (!assetCache->LoadAsset(shaderFile, asset))
		return false;

	this->mainPassShader.SafeSet(asset.Get());

	if (jsonDoc.HasMember("shadow_shader"))
	{
		std::string shadowShaderFile = jsonDoc["shadow_shader"].GetString();
		if (!assetCache->LoadAsset(shadowShaderFile, asset))
			return false;

		this->shadowPassShader.SafeSet(asset.Get());
	}

	if (jsonDoc.HasMember("index_buffer"))
	{
		std::string indexBufferFile = jsonDoc["index_buffer"].GetString();
		
		if (!assetCache->LoadAsset(indexBufferFile, asset))
			return false;

		this->indexBuffer.SafeSet(asset.Get());
	}

	if (jsonDoc.HasMember("texture"))
	{
		std::string textureFile = jsonDoc["texture"].GetString();

		if (!assetCache->LoadAsset(textureFile, asset))
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