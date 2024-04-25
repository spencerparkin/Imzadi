#include "Canvas.h"
#include "App.h"
#include "System.h"
#include "Query.h"
#include "Result.h"
#include <gl/GLU.h>
#include <wx/utils.h>

using namespace Collision;

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

Canvas::Canvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->renderContext = new wxGLContext(this);

	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);

	this->camera.SetCameraPosition(Vector3(20.0, 10.0, 20.0));
	this->camera.SetCameraTarget(Vector3(0.0, 0.0, 0.0));
}

/*virtual*/ Canvas::~Canvas()
{
	delete this->renderContext;
}

void Canvas::OnPaint(wxPaintEvent& event)
{
	this->SetCurrent(*this->renderContext);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	renderQuery->SetDrawFlags(COLL_SYS_DRAW_FLAG_SHAPES);
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

	glFlush();

	this->SwapBuffers();
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

	double leftThumbSensativity = 0.8;
	double rightThumbSensativity = 0.1;

	Vector3 leftStickVector = this->controller.GetAnalogJoyStick(Controller::JoyStick::LEFT);
	Vector3 rightStickVector = this->controller.GetAnalogJoyStick(Controller::JoyStick::RIGHT);

	Vector3 xAxis, yAxis, zAxis;
	this->camera.GetCameraFrame(xAxis, yAxis, zAxis);

	Vector3 cameraPosition = this->camera.GetCameraPosition();
	cameraPosition += (xAxis * leftStickVector.x - zAxis * leftStickVector.y) * leftThumbSensativity;
	this->camera.SetCameraPosition(cameraPosition);

	Quaternion pitchQuat, yawQuat;
	pitchQuat.SetFromAxisAngle(xAxis, rightStickVector.y * rightThumbSensativity);
	yawQuat.SetFromAxisAngle(Vector3(0.0, 1.0, 0.0), -rightStickVector.x * rightThumbSensativity);

	Quaternion quat = this->camera.GetCameraOrientation();
	quat = (pitchQuat * yawQuat * quat).Normalized();
	this->camera.SetCameraOrientation(quat);

	this->Refresh();
}