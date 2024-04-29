#include "Canvas.h"
#include "App.h"
#include "System.h"
#include "Query.h"
#include "Result.h"
#include <gl/GLU.h>
#include <wx/utils.h>
#include <time.h>

using namespace Collision;

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

Canvas::Canvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->targetShapes = false;
	this->targetShapeHitLine = nullptr;

	this->debugDrawFlags = COLL_SYS_DRAW_FLAG_SHAPES;

	this->strafeMode = StrafeMode::XZ_PLANE;
	this->renderTimeArrayMax = 32;

	this->renderContext = new wxGLContext(this);

	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);

	this->camera.SetCameraPosition(Vector3(20.0, 10.0, 20.0));
	this->camera.SetCameraTarget(Vector3(0.0, 0.0, 0.0));
}

/*virtual*/ Canvas::~Canvas()
{
	delete this->renderContext;
	delete this->targetShapeHitLine;
}

void Canvas::OnPaint(wxPaintEvent& event)
{
	clock_t startTimeTicks = ::clock();

	this->SetCurrent(*this->renderContext);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	double aspectRatio = double(viewport[2]) / double(viewport[3]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, aspectRatio, 0.1, 1000.0);

	Camera::ViewingParameters viewingParams;
	this->camera.GetViewingParameters(viewingParams);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		viewingParams.eyePoint.x,
		viewingParams.eyePoint.y,
		viewingParams.eyePoint.z,
		viewingParams.lookAtPoint.x,
		viewingParams.lookAtPoint.y,
		viewingParams.lookAtPoint.z,
		viewingParams.upVector.x,
		viewingParams.upVector.y,
		viewingParams.upVector.z
	);

	glBegin(GL_LINES);

	glColor3d(1.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(10.0, 0.0, 0.0);

	glColor3d(0.0, 1.0, 0.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 10.0, 0.0);

	glColor3d(0.0, 0.0, 1.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 10.0);

	glEnd();

	System* system = wxGetApp().GetCollisionSystem();

	auto renderQuery = system->Create<DebugRenderQuery>();
	renderQuery->SetDrawFlags(this->debugDrawFlags);
	TaskID taskID = 0;
	system->MakeQuery(renderQuery, taskID);
	system->FlushAllTasks();
	DebugRenderResult* renderResult = (DebugRenderResult*)system->ObtainQueryResult(taskID);
	if (renderResult)
	{
		glBegin(GL_LINES);

		const std::vector<DebugRenderResult::RenderLine>& renderLineArray = renderResult->GetRenderLineArray();
		for (auto renderLine : renderLineArray)
		{
			glColor3d(renderLine.color.x, renderLine.color.y, renderLine.color.z);
			glVertex3d(renderLine.line.point[0].x, renderLine.line.point[0].y, renderLine.line.point[0].z);
			glVertex3d(renderLine.line.point[1].x, renderLine.line.point[1].y, renderLine.line.point[1].z);
		}

		glEnd();

		system->Free<Result>(renderResult);
	}

	if (this->targetShapes)
	{
		// Draw a cross-hair center screen.

		Transform cameraToWorld = this->camera.GetCameraToWorldTransform();

		Vector3 points[4];
		points[0].SetComponents(-0.5 , 0.0, -10.0);
		points[1].SetComponents(0.5, 0.0, -10.0);
		points[2].SetComponents(0.0, -0.5, -10.0);
		points[3].SetComponents(0.0, 0.5, -10.0);

		for (int i = 0; i < 4; i++)
			points[i] = cameraToWorld.TransformPoint(points[i]);

		glBegin(GL_LINES);
		glColor3d(0.5, 0.5, 0.5);
		for (int i = 0; i < 4; i++)
			glVertex3d(points[i].x, points[i].y, points[i].z);
		glEnd();

		// Draw a line in space representing the hit-normal and hit-point, if any.

		if (this->targetShapeHitLine)
		{
			glBegin(GL_LINES);
			glColor3d(1.0, 1.0, 1.0);
			glVertex2dv(&this->targetShapeHitLine->point[0].x);
			glVertex2dv(&this->targetShapeHitLine->point[1].x);
			glEnd();
		}
	}

	glFlush();

	this->SwapBuffers();

	::clock_t stopTimeTicks = ::clock();
	::clock_t elapsedTimeTicks = stopTimeTicks - startTimeTicks;
	double elapsedTimeSeconds = double(elapsedTimeTicks) / double(CLOCKS_PER_SEC);
	this->renderTimeArray.push_back(elapsedTimeSeconds);
	if (this->renderTimeArray.size() > this->renderTimeArrayMax)
		this->renderTimeArray.pop_front();
}

void Canvas::OnSize(wxSizeEvent& event)
{
	this->SetCurrent(*this->renderContext);

	wxSize size = event.GetSize();
	glViewport(0, 0, size.GetWidth(), size.GetHeight());

	this->Refresh();
}

void Canvas::Tick()
{
	this->controller.Update();

	if (this->controller.ButtonPressed(XINPUT_GAMEPAD_X))
		this->targetShapes = !this->targetShapes;

	if (this->controller.ButtonPressed(XINPUT_GAMEPAD_RIGHT_SHOULDER))
	{
		switch (this->strafeMode)
		{
		case StrafeMode::XZ_PLANE:
			this->strafeMode = StrafeMode::XY_PLANE;
			break;
		case StrafeMode::XY_PLANE:
			this->strafeMode = StrafeMode::XZ_PLANE;
			break;
		}
	}

	double leftThumbSensativity = 0.5;
	double rightThumbSensativity = 0.05;

	Vector3 leftStickVector = this->controller.GetAnalogJoyStick(Controller::JoyStick::LEFT);
	Vector3 rightStickVector = this->controller.GetAnalogJoyStick(Controller::JoyStick::RIGHT);

	Vector3 xAxis, yAxis, zAxis;
	this->camera.GetCameraFrame(xAxis, yAxis, zAxis);

	Vector3 cameraPosition = this->camera.GetCameraPosition();

	Vector3 axisA, axisB;
	switch (this->strafeMode)
	{
	case StrafeMode::XZ_PLANE:
		axisA = xAxis;
		axisB = -zAxis;
		break;
	case StrafeMode::XY_PLANE:
		axisA = xAxis;
		axisB = yAxis;
		break;
	}

	cameraPosition += (axisA * leftStickVector.x + axisB * leftStickVector.y) * leftThumbSensativity;
	this->camera.SetCameraPosition(cameraPosition);

	Quaternion pitchQuat, yawQuat;
	pitchQuat.SetFromAxisAngle(xAxis, rightStickVector.y * rightThumbSensativity);
	yawQuat.SetFromAxisAngle(Vector3(0.0, 1.0, 0.0), -rightStickVector.x * rightThumbSensativity);

	Quaternion quat = this->camera.GetCameraOrientation();
	quat = (pitchQuat * yawQuat * quat).Normalized();
	this->camera.SetCameraOrientation(quat);

	if (this->targetShapes)
	{
		Ray ray;
		ray.origin.SetComponents(0.0, 0.0, 0.0);
		ray.unitDirection.SetComponents(0.0, 0.0, -1.0);

		Transform cameraToWorld = this->camera.GetCameraToWorldTransform();
		ray = cameraToWorld.TransformRay(ray);

		System* system = wxGetApp().GetCollisionSystem();

		auto rayCastQuery = system->Create<RayCastQuery>();
		rayCastQuery->SetRay(ray);

		TaskID taskID = 0;
		if (system->MakeQuery(rayCastQuery, taskID))
		{
			system->FlushAllTasks();
			auto result = (RayCastResult*)system->ObtainQueryResult(taskID);

			const RayCastResult::HitData& hitData = result->GetHitData();

			delete this->targetShapeHitLine;
			this->targetShapeHitLine = nullptr;

			if (hitData.shapeID != 0)
			{
				this->targetShapeHitLine = new LineSegment();
				this->targetShapeHitLine->point[0] = hitData.surfacePoint;
				this->targetShapeHitLine->point[1] = hitData.surfacePoint + hitData.surfaceNormal;
			}

			system->Free<RayCastResult>(result);
		}
	}

	this->Refresh();
}

double Canvas::GetAverageFramerate()
{
	double averageRenderTimeSeconds = 0.0;
	for (double renderTimeSeconds : this->renderTimeArray)
		averageRenderTimeSeconds += renderTimeSeconds;
	averageRenderTimeSeconds /= double(this->renderTimeArray.size());
	double frameRateFPS = 1.0 / averageRenderTimeSeconds;
	return frameRateFPS;
}