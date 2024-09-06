#pragma once

#include "Scene.h"
#include "Math/LineSegment.h"
#include "Assets/Shader.h"
#include <d3d11.h>

namespace Imzadi
{
	/**
	 * These are lines drawn for debug visualization purposes.
	 */
	class IMZADI_API DebugLines : public RenderObject
	{
	public:
		DebugLines();
		virtual ~DebugLines();

		virtual void Render(Camera* camera, RenderPass renderPass) override;
		virtual int SortKey() const override;

		struct Line
		{
			Vector3 color;
			LineSegment segment;
		};

		bool AddLine(const Line& line);
		bool AddBox(const AxisAlignedBoundingBox& box, const Vector3& color);
		bool AddTransform(const Transform& transform);
		void Clear();

	private:
		std::vector<Line> lineArray;
		ID3D11Buffer* vertexBuffer;
		Reference<Shader> shader;
		int maxLines;
	};
}