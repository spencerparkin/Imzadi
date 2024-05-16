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

/*virtual*/ Collision::AxisAlignedBoundingBox RenderMeshInstance::GetWorldBoundingBox() const
{
	return this->boundingBox;
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
	if (!jsonDoc.IsObject())
		return false;

	if (!jsonDoc.HasMember("vertex_buffer"))
		return false;

	std::string vertexBufferFile = jsonDoc["vertex_buffer"].GetString();
	if (!assetCache->GrabAsset(vertexBufferFile, this->vertexBuffer))
		return false;

	if (!dynamic_cast<Buffer*>(this->vertexBuffer.get()))
		return false;

	if (!jsonDoc.HasMember("shader"))
		return false;

	std::string shaderFile = jsonDoc["shader"].GetString();
	if (!assetCache->GrabAsset(shaderFile, this->shader))
		return false;

	if (jsonDoc.HasMember("index_buffer"))
	{
		std::string indexBufferFile = jsonDoc["index_buffer"].GetString();
		if (!assetCache->GrabAsset(indexBufferFile, this->indexBuffer))
			return false;
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

/*virtual*/ bool RenderMeshAsset::MakeRenderInstance(std::shared_ptr<RenderObject>& renderObject)
{
	renderObject = std::make_shared<RenderMeshInstance>();
	auto instance = dynamic_cast<RenderMeshInstance*>(renderObject.get());
	//instance->SetRenderMesh(
	// TODO: I think I want to get rid of my usage of std::shared_ptr and try to make my own reference-counting scheme.
	//       Specifically, reference-counted objects should derive from a common base class that holds the ref-count.
	//       This is nice, because it means we can convert to and from raw C-pointers, which you can't do with std::shared_ptr.

	// TODO: Write this.
	return false;
}