#include "RenderMesh.h"

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
	 */
	void DeformMesh();

private:
	Reference<BareBuffer> bindPoseVertices;
	Reference<Skeleton> skeleton;
	Reference<SkinWeights> skinWeights;
};