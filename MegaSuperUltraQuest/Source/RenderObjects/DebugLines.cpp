#include "DebugLines.h"

DebugLines::DebugLines()
{
}

/*virtual*/ DebugLines::~DebugLines()
{
}

/*virtual*/ void DebugLines::Render(Camera* camera, RenderPass renderPass)
{
	if (renderPass == RenderPass::SHADOW_PASS)
		return;

	// TODO: Write this.
}

/*virtual*/ void DebugLines::GetWorldBoundingSphere(Collision::Vector3& center, double& radius) const
{
	// TODO: Write this.
}

void DebugLines::AddLine(const Line& line)
{
	this->lineArray.push_back(line);
}

void DebugLines::Clear()
{
	this->lineArray.clear();
}