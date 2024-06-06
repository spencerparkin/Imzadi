#include "SkinnedRenderMesh.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "Skeleton.h"
#include "SkinWeights.h"

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

	if (!jsonDoc.HasMember("skeleton") || !jsonDoc["skeleton"].IsString())
		return false;

	this->skeleton.Set(new Skeleton());
	this->skeleton->MakeBasicBiped();
	this->skeleton->UpdateCachedTransforms(BoneTransformType::BIND_POSE);
	this->skeleton->ResetCurrentPose();
	this->skeleton->UpdateCachedTransforms(BoneTransformType::CURRENT_POSE);

#if 0
	std::string skeletonFile = jsonDoc["skeleton"].GetString();
	Reference<Asset> asset;
	if (!assetCache->LoadAsset(skeletonFile, asset))
		return false;

	this->skeleton.SafeSet(asset.Get());
	if (!this->skeleton)
		return false;

	if (!this->vertexBuffer->GetBareBuffer(this->bindPoseVertices))
		return false;

	if (!jsonDoc.HasMember("skin_weights") || !jsonDoc["skin_weights"].IsString())
		return false;

	std::string skinWeightsFile = jsonDoc["skin_weights"].GetString();
	if (!assetCache->LoadAsset(skinWeightsFile, asset))
		return false;

	this->skinWeights.SafeSet(asset.Get());
	if (!skinWeights)
		return false;
#endif

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