#pragma once

#include "RenderMeshInstance.h"

class AnimatedMeshInstance : public RenderMeshInstance
{
public:
	AnimatedMeshInstance();
	virtual ~AnimatedMeshInstance();

	virtual void Render(Camera* camera, RenderPass renderPass) override;
};