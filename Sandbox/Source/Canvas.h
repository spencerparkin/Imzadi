#pragma once

#include "Camera.h"
#include "Controller.h"
#include <wx/glcanvas.h>

class Canvas : public wxGLCanvas
{
public:
	Canvas(wxWindow* parent);
	virtual ~Canvas();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

	void Tick();

private:
	wxGLContext* renderContext;
	static int attributeList[];
	Camera camera;
	Controller controller;
};