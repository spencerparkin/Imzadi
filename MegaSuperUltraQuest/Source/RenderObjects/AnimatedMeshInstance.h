#pragma once

#include "RenderMeshInstance.h"
#include "Assets/Animation.h"
#include "Assets/SkinnedRenderMesh.h"

class AnimatedMeshInstance : public RenderMeshInstance
{
public:
	AnimatedMeshInstance();
	virtual ~AnimatedMeshInstance();

	virtual void Render(Camera* camera, RenderPass renderPass) override;

	void SetTransitionTime(double transitionTime) { this->transitionTime = transitionTime; }
	double GetTransitionTime() const { return this->transitionTime; }

	bool AdvanceAnimation(double deltaTime);
	bool SetAnimation(const std::string& animationName);
	Animation* GetAnimation() { return this->animation.Get(); }

	void SetSkinnedMesh(SkinnedRenderMesh* skinnedMesh) { this->skinnedMesh.Set(skinnedMesh); }
	SkinnedRenderMesh* GetSkinnedMesh() { return this->skinnedMesh.Get(); }

private:
	double transitionTime;
	double currentTransitionTime;
	KeyFrame transitionalKeyFrame;
	KeyFrame currentKeyFrame;
	Reference<Animation> animation;
	Animation::Cursor cursor;
	Reference<SkinnedRenderMesh> skinnedMesh;
};