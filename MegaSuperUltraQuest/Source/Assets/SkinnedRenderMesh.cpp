#include "SkinnedRenderMesh.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "Skeleton.h"

using namespace Collision;

SkinnedRenderMesh::SkinnedRenderMesh()
{
}

/*virtual*/ SkinnedRenderMesh::~SkinnedRenderMesh()
{
}

/*virtual*/ bool SkinnedRenderMesh::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!RenderMeshAsset::Load(jsonDoc, assetCache))
		return false;

	// TODO: Write this.

	this->skeleton.Set(new Skeleton());

	Vector3 scale(1.0, 1.0, 1.0);
	this->skeleton->MakeBasicBiped(scale);

	return true;
}

/*virtual*/ bool SkinnedRenderMesh::Unload()
{
	RenderMeshAsset::Unload();

	return true;
}

void SkinnedRenderMesh::DeformMesh()
{
	// TODO: Write this.
}

/*virtual*/ bool SkinnedRenderMesh::MakeRenderInstance(Reference<RenderObject>& renderObject)
{
	renderObject.Set(new AnimatedMeshInstance());
	auto instance = dynamic_cast<AnimatedMeshInstance*>(renderObject.Get());
	instance->SetRenderMesh(this);
	instance->SetBoundingBox(this->objectSpaceBoundingBox);
	return true;
}