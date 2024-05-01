#pragma once

#include "Camera.h"
#include "Controller.h"
#include "Shape.h"
#include "Math/LineSegment.h"
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

	wxString GetStatusMessage();

private:

	enum ControllerSensativity
	{
		LOW,
		MEDIUM,
		HIGH
	};

	enum StrafeMode
	{
		XZ_PLANE,
		XY_PLANE
	};

	struct SensativityParams
	{
		double leftThumbSensativity;
		double rightThumbSensativity;
	};

	void GetSensativityParams(SensativityParams& sensativityParams);
	void SetSelectedShape(Collision::ShapeID shapeID);

	wxGLContext* renderContext;
	static int attributeList[];
	Camera camera;
	Controller controller;
	std::list<double> renderTimeArray;
	unsigned int renderTimeArrayMax;
	StrafeMode strafeMode;
	ControllerSensativity controllerSensativity;
	uint32_t debugDrawFlags;
	bool targetShapes;
	bool dragSelectedShape;
	Collision::LineSegment* targetShapeHitLine;
	Collision::ShapeID selectedShapeID;
	Collision::Transform shapeToCamera;
};