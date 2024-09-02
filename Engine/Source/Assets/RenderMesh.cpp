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
	this->lodRadius = std::numeric_limits<double>::max();
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

	this->lodRadius = std::numeric_limits<double>::max();
	if (jsonDoc.HasMember("lod_radius") && jsonDoc["lod_radius"].IsFloat())
		this->lodRadius = jsonDoc["lod_radius"].GetFloat();

	if (jsonDoc.HasMember("next_lod") && jsonDoc["next_lod"].IsString())
	{
		std::string nextLODFile = jsonDoc["next_lod"].GetString();
		if (!assetCache->LoadAsset(nextLODFile, asset))
		{
			IMZADI_LOG_ERROR("Failed to load next LOD: %s", nextLODFile.c_str());
			return false;
		}

		this->nextLOD.SafeSet(asset.Get());
		if (!this->nextLOD)
		{
			IMZADI_LOG_ERROR("Next LOD was not a render mesh.");
			return false;
		}
	}

	this->portMap.clear();
	if (jsonDoc.HasMember("port_map") && jsonDoc["port_map"].IsObject())
	{
		const rapidjson::Value& portMapValue = jsonDoc["port_map"];
		for (auto iter = portMapValue.MemberBegin(); iter != portMapValue.MemberEnd(); iter++)
		{
			std::string portName = iter->name.GetString();
			const rapidjson::Value& transformValue = iter->value;
			Transform nodeToObject;
			if (!Asset::LoadTransform(transformValue, nodeToObject))
			{
				IMZADI_LOG_ERROR("Failed to load transform for port: %s", portName.c_str());
				return false;
			}

			this->portMap.insert(std::pair(portName, nodeToObject));
		}
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
	this->nextLOD.Set(nullptr);
	this->portMap.clear();

	return true;
}

bool RenderMeshAsset::GetPort(const std::string& portName, Transform& portToObject) const
{
	PortMap::const_iterator iter = this->portMap.find(portName);
	if (iter == this->portMap.end())
		return false;

	portToObject = iter->second;
	return true;
}

/*virtual*/ bool RenderMeshAsset::MakeRenderInstance(Reference<RenderObject>& renderObject)
{
	renderObject.Set(new RenderMeshInstance());
	auto instance = dynamic_cast<RenderMeshInstance*>(renderObject.Get());

	int lodNumber = 0;
	RenderMeshAsset* mesh = this;
	while (mesh)
	{
		instance->SetRenderMesh(mesh, lodNumber++);
		mesh = mesh->nextLOD.Get();
	}

	instance->SetBoundingBox(this->objectSpaceBoundingBox);
	instance->SetObjectToWorldTransform(this->objectToWorld);
	return true;
}