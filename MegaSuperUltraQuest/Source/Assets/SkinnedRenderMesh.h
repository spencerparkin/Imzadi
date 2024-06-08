#pragma once

#include "RenderMesh.h"
#include "Animation.h"
#include <unordered_map>

class Skeleton;
class SkinWeights;

/**
 * This is RenderMeshAsset except that the vertex buffer should be writable, and
 * we store along with it a T-pose (or original) version of the vertex buffer that
 * is read-only and paired with vertex weights, and a skeleton.
 */
class SkinnedRenderMesh : public RenderMeshAsset
{
public:
	SkinnedRenderMesh();
	virtual ~SkinnedRenderMesh();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;
	virtual bool MakeRenderInstance(Reference<RenderObject>& renderObject) override;

	Skeleton* GetSkeleton() { return this->skeleton.Get(); }

	/**
	 * This is where we write the vertex buffer (that will be sent to the GPU for rendering)
	 * as a function of the the bind-pose vertex buffer, the skeleton, and the skin-weights.
	 * 
	 * Note that here we assume that all cached transforms of the skeleton are correct.
	 */
	void DeformMesh();

	Animation* GetAnimation(const std::string& animationName);

private:
	Reference<BareBuffer> bindPoseVertices;
	Reference<BareBuffer> currentPoseVertices;
	Reference<Skeleton> skeleton;
	Reference<SkinWeights> skinWeights;
	uint32_t positionOffset;
	uint32_t normalOffset;
	typedef std::unordered_map<std::string, Reference<Animation>> AnimationMap;
	AnimationMap animationMap;
};