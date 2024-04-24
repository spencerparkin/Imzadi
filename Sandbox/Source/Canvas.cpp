#include "Canvas.h"
#include "App.h"
#include <gl/GLU.h>
#include "System.h"
#include "Query.h"
#include "Result.h"

using namespace Collision;

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

Canvas::Canvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->renderContext = new wxGLContext(this);

	this->Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
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

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 30.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

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