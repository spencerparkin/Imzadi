#include "RenderMesh.h"
#include "Camera.h"
#include "Scene.h"
#include "Buffer.h"
#include "Game.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector4.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "Log.h"

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
	{
		IMZADI_LOG_ERROR("Given JSON doc was not an object.");
		return false;
	}

	if (jsonDoc.HasMember("bounding_box"))
	{
		if (!this->LoadBoundingBox(jsonDoc["bounding_box"], this->objectSpaceBoundingBox))
		{
			IMZADI_LOG_ERROR("Failed to load \"bounding_box\" member.");
			return false;
		}
	}

	if (!jsonDoc.HasMember("vertex_buffer"))
	{
		IMZADI_LOG_ERROR("No \"vertex_buffer\" member given.");
		return false;
	}

	std::string vertexBufferFile = jsonDoc["vertex_buffer"].GetString();
	if (!assetCache->LoadAsset(vertexBufferFile, asset))
	{
		IMZADI_LOG_ERROR("Failed to load vertex buffer: " + vertexBufferFile);
		return false;
	}

	this->vertexBuffer.SafeSet(asset.Get());

	if (!jsonDoc.HasMember("shader"))
	{
		IMZADI_LOG_ERROR("No \"shader\" member given.");
		return false;
	}

	std::string shaderFile = jsonDoc["shader"].GetString();
	if (!assetCache->LoadAsset(shaderFile, asset))
	{
		IMZADI_LOG_ERROR("Failed to load shader: " + shaderFile);
		return false;
	}

	this->mainPassShader.SafeSet(asset.Get());

	if (jsonDoc.HasMember("shadow_shader"))
	{
		std::string shadowShaderFile = jsonDoc["shadow_shader"].GetString();
		if (!assetCache->LoadAsset(shadowShaderFile, asset))
		{
			IMZADI_LOG_ERROR("Failed to loaod shadow shader: " + shadowShaderFile);
			return false;
		}

		this->shadowPassShader.SafeSet(asset.Get());
	}

	if (jsonDoc.HasMember("index_buffer"))
	{
		std::string indexBufferFile = jsonDoc["index_buffer"].GetString();
		
		if (!assetCache->LoadAsset(indexBufferFile, asset))
		{
			IMZADI_LOG_ERROR("Failed to load index buffer: " + indexBufferFile);
			return false;
		}

		this->indexBuffer.SafeSet(asset.Get());
	}

	if (jsonDoc.HasMember("texture"))
	{
		std::string textureFile = jsonDoc["texture"].GetString();

		if (!assetCache->LoadAsset(textureFile, asset))
		{
			IMZADI_LOG_ERROR("Failed to load texture: " + textureFile);
			return false;
		}

		this->texture.SafeSet(asset.Get());
	}

	if (!jsonDoc.HasMember("primitive_type"))
	{
		IMZADI_LOG_ERROR("No \"primitive_type\" given.");
		return false;
	}
	
	std::string primTypeStr = jsonDoc["primitive_type"].GetString();
	if (primTypeStr == "TRIANGLE_LIST")
		this->primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	if (this->primType == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
	{
		IMZADI_LOG_ERROR(std::format("Primitive type \"{}\" not yet supported or is unrecognized.", primTypeStr.c_str()));
		return false;
	}

	if (!jsonDoc.HasMember("object_to_world"))
		this->objectToWorld.SetIdentity();
	else if (!Asset::LoadTransform(jsonDoc["object_to_world"], this->objectToWorld))
	{
		IMZADI_LOG_ERROR("Failed to load object-to-world transform.");
		return false;
	}

	return true;
}

/*virtual*/ bool RenderMeshAsset::Unload()
{
	this->vertexBuffer.Set(nullptr);
	this->indexBuffer.Set(nullptr);
	this->mainPassShader.Set(nullptr);
	this->shadowPassShader.Set(nullptr);
	this->texture.Set(nullptr);

	return true;
}

/*virtual*/ bool RenderMeshAsset::MakeRenderInstance(Reference<RenderObject>& renderObject)
{
	renderObject.Set(new RenderMeshInstance());
	auto instance = dynamic_cast<RenderMeshInstance*>(renderObject.Get());
	instance->SetRenderMesh(this);
	instance->SetBoundingBox(this->objectSpaceBoundingBox);
	instance->SetObjectToWorldTransform(this->objectToWorld);
	return true;
}