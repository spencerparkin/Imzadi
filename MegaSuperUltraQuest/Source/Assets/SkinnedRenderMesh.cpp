#include "SkinnedRenderMesh.h"

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