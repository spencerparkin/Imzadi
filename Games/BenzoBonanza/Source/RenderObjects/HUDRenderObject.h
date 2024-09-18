#pragma once

#include "RenderObjects/TextRenderObject.h"

class HUDRenderObject : public Imzadi::TextRenderObject
{
public:
	HUDRenderObject();
	virtual ~HUDRenderObject();

	virtual void Prepare() override;
};