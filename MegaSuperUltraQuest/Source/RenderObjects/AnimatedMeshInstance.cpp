#include "AnimatedMeshInstance.h"
#include "Assets/SkinnedRenderMesh.h"
#include "Assets/Skeleton.h"

AnimatedMeshInstance::AnimatedMeshInstance()
{
}

/*virtual*/ AnimatedMeshInstance::~AnimatedMeshInstance()
{
}

/*virtual*/ void AnimatedMeshInstance::Render(Camera* camera, RenderPass renderPass)
{
	RenderMeshInstance::Render(camera, renderPass);

	if (renderPass == RenderPass::MAIN_PASS)
	{
		auto skinnedMesh = dynamic_cast<SkinnedRenderMesh*>(this->mesh.Get());
		if (skinnedMesh)
		{
			Skeleton* skeleton = skinnedMesh->GetSkeleton();
			if (skeleton)
				skeleton->DebugDraw(BoneTransformType::CURRENT_POSE, this->objectToWorld);
		}
	}
}