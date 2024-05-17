#include "RenderMesh.h"
#include "Camera.h"
#include "Scene.h"
#include "Buffer.h"

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

	// TODO: Build the object to projection space matrix here so the shader has it.

	// TODO: Write this.
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
	// TODO: Write this.
	return false;
}

/*virtual*/ bool RenderMeshAsset::MakeRenderInstance(Reference<RenderObject>& renderObject)
{
	renderObject.Set(new RenderMeshInstance());
	auto instance = dynamic_cast<RenderMeshInstance*>(renderObject.Get());
	instance->SetRenderMesh(this);
	instance->SetBoundingBox(this->objectSpaceBoundingBox);
	return true;
}