#pragma once

#include "Scene.h"
#include "Math/LineSegment.h"

/**
 * These are lines drawn for debug visualization purposes.
 */
class DebugLines : public RenderObject
{
public:
	DebugLines();
	virtual ~DebugLines();

	virtual void Render(Camera* camera, RenderPass renderPass) override;
	virtual void GetWorldBoundingSphere(Collision::Vector3& center, double& radius) const override;

	struct Line
	{
		Collision::Vector3 color;
		Collision::LineSegment segment;
	};

	void AddLine(const Line& line);
	void Clear();

private:
	std::vector<Line> lineArray;
	// TODO: Own CPU-accessable vertex buffer here.
	// TODO: Own line shaders.
};