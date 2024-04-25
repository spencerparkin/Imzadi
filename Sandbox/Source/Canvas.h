#pragma once

#include "Camera.h"
#include "Controller.h"
#include <wx/glcanvas.h>
#include <list>

class Canvas : public wxGLCanvas
{
public:
	Canvas(wxWindow* parent);
	virtual ~Canvas();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

	void Tick();

	double GetAverageFramerate();

private:
	wxGLContext* renderContext;
	static int attributeList[];
	Camera camera;
	Controller controller;
	std::list<double> renderTimeArray;
	unsigned int renderTimeArrayMax;
};