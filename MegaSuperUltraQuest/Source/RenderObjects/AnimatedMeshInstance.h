#pragma once

#include "RenderMeshInstance.h"

class AnimatedMeshInstance : public RenderMeshInstance
{
public:
	AnimatedMeshInstance();
	virtual ~AnimatedMeshInstance();

	virtual void Render(Camera* camera, RenderPass renderPass) override;

	// TODO: Own set of animations, a cursor for each, and an API for easily
	//       playing animations by name at any time, and do all the logic for
	//       smoothly blending from one animation to another when the current
	//       animation changes.
};