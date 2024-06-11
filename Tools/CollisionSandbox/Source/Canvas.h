#pragma once

#include <wx/app.h>
#include "Camera.h"
#include "Controller.h"
#include "Collision/Shape.h"
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
	void OnKeyPressed(wxKeyEvent& event);

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
	void SetSelectedShape(Imzadi::ShapeID shapeID);

	wxGLContext* renderContext;
	static int attributeList[];
	Camera camera;
	Imzadi::Controller controller;
	std::list<double> renderTimeArray;
	unsigned int renderTimeArrayMax;
	StrafeMode strafeMode;
	ControllerSensativity controllerSensativity;
	uint32_t debugDrawFlags;
	bool targetShapes;
	bool dragSelectedShape;
	Imzadi::LineSegment* targetShapeHitLine;
	Imzadi::ShapeID selectedShapeID;
	Imzadi::Transform shapeToCamera;
};