#include "AnimatedMeshInstance.h"
#include "Assets/SkinnedRenderMesh.h"
#include "Assets/Skeleton.h"

using namespace Imzadi;

bool AnimatedMeshInstance::renderSkeletons = false;

/*static*/ void AnimatedMeshInstance::SetRenderSkeletons(bool render)
{
	renderSkeletons = render;
}

/*static*/ bool AnimatedMeshInstance::GetRenderSkeletons()
{
	return renderSkeletons;
}

AnimatedMeshInstance::AnimatedMeshInstance()
{
	this->transitionTime = 0.2;
	this->currentTransitionTime = 0.0;
	cursor.i = 0;
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
		if (renderSkeletons)
		{
			Skeleton* skeleton = this->skinnedMesh->GetSkeleton();
			if (skeleton)
				skeleton->DebugDraw(BoneTransformType::CURRENT_POSE, this->objectToWorld);
		}
	}
}

bool AnimatedMeshInstance::SetAnimation(const std::string& animationName)
{
	if (this->animation.Get() && this->animation->GetName() == animationName)
		return true;

	this->transitionalKeyFrame.Copy(this->currentKeyFrame);

	this->animation.Set(this->skinnedMesh->GetAnimation(animationName));
	if (!this->animation)
		return false;

	if (!this->animation->MakeCursorFromTime(this->cursor, this->animation->GetStartTime()))
	{
		this->animation.Set(nullptr);
		return false;
	}

	if (this->transitionalKeyFrame.GetPoseCount() > 0)
	{
		this->currentTransitionTime = this->animation->GetStartTime() - this->transitionTime;
		this->transitionalKeyFrame.SetTime(this->currentTransitionTime);
	}

	return true;
}

bool AnimatedMeshInstance::AdvanceAnimation(double deltaTime)
{
	Skeleton* skeleton = this->skinnedMesh->GetSkeleton();
	if (!skeleton)
		return false;

	if (!this->animation)
		return false;

	KeyFramePair keyFramePair{};

	if (this->transitionalKeyFrame.GetPoseCount() > 0)
	{
		this->animation->GetKeyFramesFromCursor(this->cursor, keyFramePair);

		this->currentTransitionTime += deltaTime;
		KeyFramePair transitionPair{ &this->transitionalKeyFrame, keyFramePair.lowerBound };

		if (this->currentTransitionTime < keyFramePair.lowerBound->GetTime())
			this->currentKeyFrame.Interpolate(transitionPair, this->currentTransitionTime);
		else
		{
			this->animation->AdvanceCursor(cursor, this->currentTransitionTime - keyFramePair.lowerBound->GetTime(), true);
			this->animation->GetKeyFramesFromCursor(this->cursor, keyFramePair);
			this->currentKeyFrame.Interpolate(keyFramePair, this->cursor.timeSeconds);
			this->transitionalKeyFrame.Clear();
		}
	}
	else
	{
		this->animation->AdvanceCursor(cursor, deltaTime, true);
		this->animation->GetKeyFramesFromCursor(this->cursor, keyFramePair);
		this->currentKeyFrame.Interpolate(keyFramePair, this->cursor.timeSeconds);
	}

	this->currentKeyFrame.PoseSkeleton(skeleton);
	skeleton->UpdateCachedTransforms(BoneTransformType::CURRENT_POSE);
	this->skinnedMesh->DeformMesh();
	return true;
}