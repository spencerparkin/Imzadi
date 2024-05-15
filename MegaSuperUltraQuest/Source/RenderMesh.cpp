#include "RenderMesh.h"
#include "Camera.h"
#include "Scene.h"

using namespace Collision;

RenderMesh::RenderMesh()
{
	this->objectToWorld.SetIdentity();
}

/*virtual*/ RenderMesh::~RenderMesh()
{
}

/*virtual*/ bool RenderMesh::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	//...

	return false;
}

/*virtual*/ bool RenderMesh::Unload()
{
	return false;
}

void RenderMesh::Render(Scene* scene)
{
	Camera* camera = scene->GetCamera();

	// TODO: Build the object to projection space matrix here so the shader has it.

	// TODO: Write this.
}