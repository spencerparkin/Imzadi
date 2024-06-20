#pragma once

#include <wx/textctrl.h>
#include "Scene.h"

class RenderObjectProperties : public wxTextCtrl
{
public:
	RenderObjectProperties(wxWindow* parent);
	virtual ~RenderObjectProperties();

	void PrintPropertiesOf(Imzadi::RenderObject* renderObject);
};