#include "AnimatedMeshInstance.h"
#include "Assets/SkinnedRenderMesh.h"
#include "Assets/Skeleton.h"

AnimatedMeshInstance::AnimatedMeshInstance()
{
	this->transitionTimeSeconds = 0.2;
	cursor.i = 0;
	cursor.loop = true;
	cursor.timeSeconds = 0.0;
}

/*virtual*/ AnimatedMeshInstance::~AnimatedMeshInstance()
{
}

/*virtual*/ void AnimatedMeshInstance::Render(Camera* camera, RenderPass renderPass)
{
	RenderMeshInstance::Render(camera, renderPass);

	if (renderPass == RenderPass::MAIN_PASS)
	{
		// TODO: Only do this if a debug flag is set.
		Skeleton* skeleton = this->skinnedMesh->GetSkeleton();
		if (skeleton)
			skeleton->DebugDraw(BoneTransformType::CURRENT_POSE, this->objectToWorld);
	}
}

bool AnimatedMeshInstance::SetAnimation(const std::string& animationName)
{
	// TODO: This is all crap. Rewrite/rethink it all.

	if (this->animation)
	{
		this->transitionalKeyFrame.Clear();

		const KeyFrame* keyFrameA = nullptr;
		const KeyFrame* keyFrameB = nullptr;
		if (this->animation->GetKeyFramesFromCursor(this->cursor, keyFrameA, keyFrameB))
			this->transitionalKeyFrame.Interpolate(keyFrameA, keyFrameB, this->cursor.timeSeconds);
	}

	this->animation.Set(this->skinnedMesh->GetAnimation(animationName));
	if (!this->animation)
		return false;

	this->cursor.i = 0;
	this->cursor.loop = true;
	this->cursor.timeSeconds = 0.0;

	if (this->transitionalKeyFrame.GetPoseCount() > 0)
	{
		this->transitionalKeyFrame.SetTime(-this->transitionTimeSeconds);
		this->cursor.timeSeconds = -this->transitionTimeSeconds;
	}

	return true;
}

void AnimatedMeshInstance::AdvanceAnimation(double deltaTime)
{
	Skeleton* skeleton = this->skinnedMesh->GetSkeleton();
	if (!skeleton)
		return;

	if (!this->animation)
	{
		skeleton->ResetCurrentPose();
		return;
	}

	KeyFrame keyFrame;

	if (this->transitionalKeyFrame.GetPoseCount() > 0)
	{
		//keyFrame.Interpolate(this->transitionalKeyFrame, ...)
	}
	else
	{
		const KeyFrame* keyFrameA = nullptr;
		const KeyFrame* keyFrameB = nullptr;
		if (!this->animation->GetKeyFramesFromCursor(this->cursor, keyFrameA, keyFrameB))
		{
			skeleton->ResetCurrentPose();
			return;
		}
		
		this->transitionalKeyFrame.Interpolate(keyFrameA, keyFrameB, this->cursor.timeSeconds);
	}

	keyFrame.Pose(skeleton);
	skeleton->UpdateCachedTransforms(BoneTransformType::CURRENT_POSE);
	this->skinnedMesh->DeformMesh();
}