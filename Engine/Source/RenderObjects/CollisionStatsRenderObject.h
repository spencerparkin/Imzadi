#pragma once

#include "TextRenderObject.h"

namespace Imzadi
{
	class IMZADI_API CollisionStatsRenderObject : public TextRenderObject
	{
	public:
		CollisionStatsRenderObject();
		virtual ~CollisionStatsRenderObject();

		virtual void PreRender() override;
	};
}