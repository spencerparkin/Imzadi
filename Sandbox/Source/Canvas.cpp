#include "Canvas.h"
#include "App.h"
#include "System.h"
#include "Command.h"
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
	this->controllerSensativity = ControllerSensativity::MEDIUM;
	this->strafeMode = StrafeMode::XZ_PLANE;
	this->renderTimeArrayMax = 32;
	this->selectedShapeID = 0;

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
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	glLineWidth(2.0);
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
		glLineWidth(1.0);
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
		points[0].SetComponents(-0.1 , 0.0, -1.0);
		points[1].SetComponents(0.1, 0.0, -1.0);
		points[2].SetComponents(0.0, -0.1, -1.0);
		points[3].SetComponents(0.0, 0.1, -1.0);

		for (int i = 0; i < 4; i++)
			points[i] = cameraToWorld.TransformPoint(points[i]);

		glLineWidth(2.0);
		glBegin(GL_LINES);
		glColor3d(1.0, 0.0, 1.0);
		for (int i = 0; i < 4; i++)
			glVertex3d(points[i].x, points[i].y, points[i].z);
		glEnd();

		// Draw a line in space representing the hit-normal and hit-point, if any.

		if (this->targetShapeHitLine)
		{
			glLineWidth(2.0);
			glBegin(GL_LINES);
			glColor3d(0.0, 1.0, 1.0);
			glVertex3dv(&this->targetShapeHitLine->point[0].x);
			glVertex3dv(&this->targetShapeHitLine->point[1].x);
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

void Canvas::GetSensativityParams(SensativityParams& sensativityParams)
{
	switch (this->controllerSensativity)
	{
	case ControllerSensativity::LOW:
		sensativityParams.leftThumbSensativity = 0.1;
		sensativityParams.rightThumbSensativity = 0.01;
		break;
	case ControllerSensativity::MEDIUM:
		sensativityParams.leftThumbSensativity = 0.5;
		sensativityParams.rightThumbSensativity = 0.05;
		break;
	case ControllerSensativity::HIGH:
		sensativityParams.leftThumbSensativity = 3.0;
		sensativityParams.rightThumbSensativity = 0.08;
		break;
	default:
		sensativityParams.leftThumbSensativity = 0.0;
		sensativityParams.rightThumbSensativity = 0.0;
		break;
	}
}

wxString Canvas::GetStatusMessage()
{
	wxString message;

	switch (this->strafeMode)
	{
	case StrafeMode::XZ_PLANE:
		message += "Strafe XZ";
		break;
	case StrafeMode::XY_PLANE:
		message += "Strafe XY";
		break;
	}

	switch (this->controllerSensativity)
	{
	case ControllerSensativity::LOW:
		message += " | Sensativity LOW";
		break;
	case ControllerSensativity::MEDIUM:
		message += " | Sensativity MEDIUM";
		break;
	case ControllerSensativity::HIGH:
		message += " | Sensativity HIGH";
		break;
	}

	return message;
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

	if (this->controller.ButtonPressed(XINPUT_GAMEPAD_LEFT_SHOULDER))
	{
		switch (this->controllerSensativity)
		{
		case ControllerSensativity::LOW:
			this->controllerSensativity = ControllerSensativity::MEDIUM;
			break;
		case ControllerSensativity::MEDIUM:
			this->controllerSensativity = ControllerSensativity::HIGH;
			break;
		case ControllerSensativity::HIGH:
			this->controllerSensativity = ControllerSensativity::LOW;
			break;
		}
	}

	SensativityParams sensativityParams;
	this->GetSensativityParams(sensativityParams);

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

	cameraPosition += (axisA * leftStickVector.x + axisB * leftStickVector.y) * sensativityParams.leftThumbSensativity;
	this->camera.SetCameraPosition(cameraPosition);

	Quaternion pitchQuat, yawQuat;
	pitchQuat.SetFromAxisAngle(xAxis, rightStickVector.y * sensativityParams.rightThumbSensativity);
	yawQuat.SetFromAxisAngle(Vector3(0.0, 1.0, 0.0), -rightStickVector.x * sensativityParams.rightThumbSensativity);

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
		ShapeID hitShapeID = 0;
		if (system->MakeQuery(rayCastQuery, taskID))
		{
			system->FlushAllTasks();
			auto result = (RayCastResult*)system->ObtainQueryResult(taskID);

			const RayCastResult::HitData& hitData = result->GetHitData();
			hitShapeID = hitData.shapeID;

			delete this->targetShapeHitLine;
			this->targetShapeHitLine = nullptr;

			if (hitShapeID != 0)
			{
				this->targetShapeHitLine = new LineSegment();
				this->targetShapeHitLine->point[0] = hitData.surfacePoint;
				this->targetShapeHitLine->point[1] = hitData.surfacePoint + 3.0 * hitData.surfaceNormal;
			}

			system->Free<RayCastResult>(result);
		}

		if (this->controller.ButtonPressed(XINPUT_GAMEPAD_Y))
		{
			this->SetSelectedShape(hitShapeID);
		}
	}

	this->Refresh();
}

void Canvas::SetSelectedShape(Collision::ShapeID shapeID)
{
	if (shapeID != this->selectedShapeID)
	{
		System* system = wxGetApp().GetCollisionSystem();

		if (this->selectedShapeID != 0)
		{
			auto setColorCommand = system->Create<SetDebugRenderColorCommand>();
			setColorCommand->SetColor(Vector3(1.0, 0.0, 0.0));
			setColorCommand->SetShapeID(this->selectedShapeID);
			system->IssueCommand(setColorCommand);
		}

		this->selectedShapeID = shapeID;

		if (this->selectedShapeID != 0)
		{
			auto setColorCommand = system->Create<SetDebugRenderColorCommand>();
			setColorCommand->SetColor(Vector3(0.0, 1.0, 0.0));
			setColorCommand->SetShapeID(this->selectedShapeID);
			system->IssueCommand(setColorCommand);
		}
	}
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