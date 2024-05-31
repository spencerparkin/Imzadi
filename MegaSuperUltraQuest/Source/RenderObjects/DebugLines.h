#pragma once

#include "Scene.h"
#include "Math/LineSegment.h"
#include "Assets/Shader.h"
#include <d3d11.h>

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

	bool AddLine(const Line& line);
	void Clear();

private:
	std::vector<Line> lineArray;
	ID3D11Buffer* vertexBuffer;
	Reference<Shader> shader;
	int maxLines;
};