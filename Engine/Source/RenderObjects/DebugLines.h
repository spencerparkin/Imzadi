#pragma once

#include "Scene.h"
#include "Math/LineSegment.h"
#include "Assets/Shader.h"
#include <d3d12.h>

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
		virtual void GetWorldBoundingSphere(Imzadi::Vector3& center, double& radius) const override;
		virtual int SortKey() const override;
		virtual void Prepare() override;

		struct Line
		{
			Vector3 color;
			LineSegment segment;
		};

		bool AddLine(const Line& line);
		void Clear();

	private:
		std::vector<Line> lineArray;
#if 0
		ID3D11Buffer* vertexBuffer;
#endif
		Reference<Shader> shader;
		int maxLines;
	};
}