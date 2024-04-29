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

	uint32_t GetDebugDrawFlags() const { return this->debugDrawFlags; }
	void SetDebugDrawFlags(uint32_t drawFlags) { this->debugDrawFlags = drawFlags; }

private:

	enum StrafeMode
	{
		XZ_PLANE,
		XY_PLANE
	};

	wxGLContext* renderContext;
	static int attributeList[];
	Camera camera;
	Controller controller;
	std::list<double> renderTimeArray;
	unsigned int renderTimeArrayMax;
	StrafeMode strafeMode;
	uint32_t debugDrawFlags;
	bool targetShapes;
};