#include "Canvas.h"
#include "App.h"
#include "Collision/System.h"
#include "Collision/Command.h"
#include "Collision/Query.h"
#include "Collision/Result.h"
#include "Collision/CollisionCache.h"
#include <gl/GLU.h>
#include <wx/utils.h>
#include <time.h>

using namespace Imzadi;

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

Canvas::Canvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize), controller(0)
{
	this->targetShapes = false;
	this->targetShapeHitLine = nullptr;
	this->debugDrawFlags = IMZADI_DRAW_FLAG_SHAPES;
	this->controllerSensativity = ControllerSensativity::MEDIUM;
	this->strafeMode = StrafeMode::XZ_PLANE;
	this->renderTimeArrayMax = 32;
	this->selectedShapeID = 0;
	this->dragSelectedShape = false;

	this->renderContext = new wxGLContext(this);

	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
	this->Bind(wxEVT_KEY_DOWN, &Canvas::OnKeyPressed, this);

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

	const std::vector<Imzadi::Polygon>& polygonArray = wxGetApp().GetPolygonArray();
	GLfloat red = 0.0;
	GLfloat green = 0.5;
	GLfloat blue = 0.2;
	for (int i = 0; i < (signed)polygonArray.size(); i++)
	{
		static int debug_i = -1;
		if (debug_i != -1 && debug_i != i)
			continue;

		const Imzadi::Polygon& polygon = polygonArray[i];
#if 0
		glBegin(GL_POLYGON);
#else
		glBegin(GL_LINE_LOOP);
#endif
		glColor3d(red, green, blue);
		for (const Imzadi::Vector3& vertex : polygon.vertexArray)
			glVertex3dv(&vertex.x);
		glEnd();

		red = fmodf(red + 0.2f, 1.0f);
		green = fmodf(green * 2.0f + 0.3f, 1.0f);
		blue = fmodf(blue * 0.2 + 0.7f, 1.0f);
	}

	Collision::System* system = wxGetApp().GetCollisionSystem();

	auto renderQuery = new Collision::DebugRenderQuery();
	renderQuery->SetDrawFlags(this->debugDrawFlags);
	Collision::TaskID taskID = 0;
	system->MakeQuery(renderQuery, taskID);
	system->FlushAllTasks();
	Collision::Result* result = system->ObtainQueryResult(taskID);
	auto renderResult = dynamic_cast<Collision::DebugRenderResult*>(result);
	if (renderResult)
	{
		glLineWidth(1.0);
		glBegin(GL_LINES);

		const std::vector<Collision::DebugRenderResult::RenderLine>& renderLineArray = renderResult->GetRenderLineArray();
		for (auto renderLine : renderLineArray)
		{
			glColor3d(renderLine.color.x, renderLine.color.y, renderLine.color.z);
			glVertex3d(renderLine.line.point[0].x, renderLine.line.point[0].y, renderLine.line.point[0].z);
			glVertex3d(renderLine.line.point[1].x, renderLine.line.point[1].y, renderLine.line.point[1].z);
		}

		glEnd();
	}

	delete result;

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
		sensativityParams.rightThumbSensativity = 0.02;
		break;
	case ControllerSensativity::HIGH:
		sensativityParams.leftThumbSensativity = 3.0;
		sensativityParams.rightThumbSensativity = 0.03;
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

void Canvas::OnKeyPressed(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case WXK_DELETE:
		{
			if (this->selectedShapeID != 0)
			{
				Collision::System* system = wxGetApp().GetCollisionSystem();
				system->RemoveShape(this->selectedShapeID);
				this->selectedShapeID = 0;
				this->Refresh();
			}

			break;
		}
	}
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

	Vector3 leftStickVector;
	this->controller.GetAnalogJoyStick(Controller::Side::LEFT, leftStickVector.x, leftStickVector.y);

	Vector3 rightStickVector;
	this->controller.GetAnalogJoyStick(Controller::Side::RIGHT, rightStickVector.x, rightStickVector.y);

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

	// TODO: No, we need to make sure the up-direction of the camera is accurate at all times.
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

		Collision::System* system = wxGetApp().GetCollisionSystem();

		auto rayCastQuery = new Collision::RayCastQuery();
		rayCastQuery->SetRay(ray);

		Collision::TaskID taskID = 0;
		Collision::ShapeID hitShapeID = 0;
		if (system->MakeQuery(rayCastQuery, taskID))
		{
			system->FlushAllTasks();

			Collision::Result* result = system->ObtainQueryResult(taskID);
			auto rayCastResult = dynamic_cast<Collision::RayCastResult*>(result);
			if (rayCastResult)
			{
				const Collision::RayCastResult::HitData& hitData = rayCastResult->GetHitData();
				hitShapeID = hitData.shapeID;

				delete this->targetShapeHitLine;
				this->targetShapeHitLine = nullptr;

				if (hitShapeID != 0)
				{
					this->targetShapeHitLine = new LineSegment();
					this->targetShapeHitLine->point[0] = hitData.surfacePoint;
					this->targetShapeHitLine->point[1] = hitData.surfacePoint + 3.0 * hitData.surfaceNormal;
				}
			}

			delete result;
		}

		if (this->controller.ButtonPressed(XINPUT_GAMEPAD_Y))
		{
			this->SetSelectedShape(hitShapeID);
		}
	}

	if (this->selectedShapeID != 0)
	{
		bool dpadDown = this->controller.ButtonDown(XINPUT_GAMEPAD_DPAD_DOWN);
		bool dpadUp = this->controller.ButtonDown(XINPUT_GAMEPAD_DPAD_UP);
		bool dpadLeft = this->controller.ButtonDown(XINPUT_GAMEPAD_DPAD_LEFT);
		bool dpadRight = this->controller.ButtonDown(XINPUT_GAMEPAD_DPAD_RIGHT);

		if (dpadDown || dpadUp || dpadLeft || dpadRight)
		{
			Collision::System* system = wxGetApp().GetCollisionSystem();

			auto query = new Collision::ObjectToWorldQuery();
			query->SetShapeID(this->selectedShapeID);
			Collision::TaskID taskID = 0;
			if (system->MakeQuery(query, taskID))
			{
				system->FlushAllTasks();

				Collision::Result* result = system->ObtainQueryResult(taskID);
				auto objectToWorldResult = dynamic_cast<Collision::ObjectToWorldResult*>(result);
				if (objectToWorldResult)
				{
					Transform shapeToWorld = objectToWorldResult->objectToWorld;
					Transform worldToShape;
					worldToShape.Invert(shapeToWorld);

					const Transform& cameraToWorld = this->camera.GetCameraToWorldTransform();

					Vector3 xAxis, yAxis, zAxis;
					cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);
					xAxis = worldToShape.TransformVector(xAxis);
					yAxis = worldToShape.TransformVector(yAxis);

					double xScale = (dpadDown ? 1.0 : (dpadUp ? -1.0 : 0.0));
					double yScale = (dpadRight ? 1.0 : (dpadLeft ? -1.0 : 0.0));

					constexpr double rotationSpeed = 0.01;
					double xAngle = xScale * rotationSpeed;
					double yAngle = yScale * rotationSpeed;

					Transform xAxisRotation, yAxisRotation;
					xAxisRotation.matrix.SetFromAxisAngle(xAxis, xAngle);
					yAxisRotation.matrix.SetFromAxisAngle(yAxis, yAngle);

					shapeToWorld = shapeToWorld * xAxisRotation * yAxisRotation;
					shapeToWorld.matrix.Orthonormalized(IMZADI_AXIS_FLAG_X);

					auto command = new Collision::ObjectToWorldCommand();
					command->objectToWorld = shapeToWorld;
					command->SetShapeID(this->selectedShapeID);
					system->IssueCommand(command);
				}

				delete result;
			}
		}

		if (this->controller.ButtonPressed(XINPUT_GAMEPAD_A))
		{
			this->dragSelectedShape = !this->dragSelectedShape;

			if (this->dragSelectedShape)
			{
				Collision::System* system = wxGetApp().GetCollisionSystem();

				auto query = new Collision::ObjectToWorldQuery();
				query->SetShapeID(this->selectedShapeID);
				Collision::TaskID taskID = 0;
				if (system->MakeQuery(query, taskID))
				{
					system->FlushAllTasks();

					Collision::Result* result = system->ObtainQueryResult(taskID);
					auto objectToWorldResult = dynamic_cast<Collision::ObjectToWorldResult*>(result);
					if (objectToWorldResult)
					{
						const Transform& shapeToWorld = objectToWorldResult->objectToWorld;
						const Transform& cameraToWorld = this->camera.GetCameraToWorldTransform();

						Transform worldToCamera;
						worldToCamera.Invert(cameraToWorld);
						this->shapeToCamera = worldToCamera * shapeToWorld;
					}

					delete result;
				}
			}
		}

		if (this->dragSelectedShape)
		{
			Collision::System* system = wxGetApp().GetCollisionSystem();
			auto command = new Collision::ObjectToWorldCommand();
			command->objectToWorld = this->camera.GetCameraToWorldTransform() * this->shapeToCamera;
			command->SetShapeID(this->selectedShapeID);
			system->IssueCommand(command);
		}

		if (this->controller.ButtonPressed(XINPUT_GAMEPAD_B))
		{
			Collision::System* system = wxGetApp().GetCollisionSystem();

			auto collisionQuery = new Collision::CollisionQuery();
			collisionQuery->SetShapeID(this->selectedShapeID);
			Collision::TaskID taskID = 0;
			if (system->MakeQuery(collisionQuery, taskID))
			{
				system->FlushAllTasks();
				Collision::Result* result = system->ObtainQueryResult(taskID);
				auto collisionResult = dynamic_cast<Collision::CollisionQueryResult*>(result);
				if (collisionResult)
				{
					const Collision::ShapePairCollisionStatus* collisionStatus = collisionResult->GetMostEgregiousCollision();
					if(collisionStatus)
					{
						Transform resolverTransform;
						resolverTransform.SetIdentity();
						resolverTransform.translation = collisionStatus->GetSeparationDelta(this->selectedShapeID);
						auto command = new Collision::ObjectToWorldCommand();
						command->SetShapeID(this->selectedShapeID);
						command->objectToWorld = resolverTransform * collisionResult->GetObjectToWorldTransform();
						system->IssueCommand(command);
					}
				}

				delete result;
			}
		}
	}

	this->Refresh();
}

void Canvas::SetSelectedShape(Imzadi::Collision::ShapeID shapeID)
{
	if (shapeID != this->selectedShapeID)
	{
		this->dragSelectedShape = false;

		Collision::System* system = wxGetApp().GetCollisionSystem();

		if (this->selectedShapeID != 0)
		{
			auto setColorCommand = new Collision::SetDebugRenderColorCommand();
			setColorCommand->SetColor(Vector3(1.0, 0.0, 0.0));
			setColorCommand->SetShapeID(this->selectedShapeID);
			system->IssueCommand(setColorCommand);
		}

		this->selectedShapeID = shapeID;

		if (this->selectedShapeID != 0)
		{
			auto setColorCommand = new Collision::SetDebugRenderColorCommand();
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