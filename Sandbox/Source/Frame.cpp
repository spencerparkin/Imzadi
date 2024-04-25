#include "Frame.h"
#include "App.h"
#include "Canvas.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/aboutdlg.h>
#include "Shapes/Box.h"
#include "Shapes/Capsule.h"
#include "Shapes/Polygon.h"
#include "Shapes/Sphere.h"

using namespace Collision;

Frame::Frame(const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY, "Sandbox", pos, size), timer(this, ID_Timer)
{
	this->canvas = nullptr;

	wxMenu* programMenu = new wxMenu();
	programMenu->Append(new wxMenuItem(programMenu, ID_Exit, "Exit", "Go skiing."));

	wxMenu* shapeMenu = new wxMenu();
	shapeMenu->Append(new wxMenuItem(shapeMenu, ID_AddBox, "Add Box", "Add a box shape to the collision world."));
	shapeMenu->Append(new wxMenuItem(shapeMenu, ID_AddCapsule, "Add Capsule", "Add a capsule shape to the collision world."));
	shapeMenu->Append(new wxMenuItem(shapeMenu, ID_AddPolygon, "Add Polygon", "Add a polygon shape to the collision world."));
	shapeMenu->Append(new wxMenuItem(shapeMenu, ID_AddSphere, "Add Sphere", "Add a sphere shape to the collision world."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(programMenu, "Program");
	menuBar->Append(shapeMenu, "Shape");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));

	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnAddShape, this, ID_AddBox);
	this->Bind(wxEVT_MENU, &Frame::OnAddShape, this, ID_AddCapsule);
	this->Bind(wxEVT_MENU, &Frame::OnAddShape, this, ID_AddPolygon);
	this->Bind(wxEVT_MENU, &Frame::OnAddShape, this, ID_AddSphere);
	this->Bind(wxEVT_TIMER, &Frame::OnTimer, this, ID_Timer);

	this->canvas = new Canvas(this);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(this->canvas, 1, wxALL | wxGROW, 0);
	this->SetSizer(sizer);

	this->inTimer = false;
	this->timer.Start(0);
}

/*virtual*/ Frame::~Frame()
{
}

void Frame::OnTimer(wxTimerEvent& event)
{
	if (this->inTimer)
		return;

	this->inTimer = true;

	this->canvas->Tick();

	static int tickCount = 0;
	if (tickCount++ % 16 == 0)
	{
		double frameRateFPS = this->canvas->GetAverageFramerate();
		this->GetStatusBar()->SetStatusText(wxString::Format("Frame-rate: %1.2f FPS", frameRateFPS));
	}

	this->inTimer = false;
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

// TODO: It should be possible to click on a shape to move it with the mouse.
//       This is a good test of the ray-cast query capability of the collision system.
void Frame::OnAddShape(wxCommandEvent& event)
{
	System* system = wxGetApp().GetCollisionSystem();
	Shape* shape = nullptr;

	// TODO: Maybe ask the user for size information?
	switch (event.GetId())
	{
		case ID_AddBox:
		{
			auto box = system->Create<BoxShape>();
			box->SetExtents(Vector3(3.0, 4.0, 5.0));
			shape = box;
			break;
		}
		case ID_AddCapsule:
		{
			auto capsule = system->Create<CapsuleShape>();
			capsule->SetRadius(4.0);
			capsule->SetVertex(0, Vector3(-3.0, 0.0, 0.0));
			capsule->SetVertex(1, Vector3(3.0, 0.0, 0.0));
			shape = capsule;
			break;
		}
		case ID_AddPolygon:
		{
			auto polygon = system->Create<PolygonShape>();
			int vertexCount = 5;
			double radius = 5.0;
			polygon->SetNumVertices(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				double angle = 2.0 * M_PI * double(i) / double(vertexCount);
				Vector3 vertex;
				vertex.x = radius * ::cos(angle);
				vertex.y = radius * ::sin(angle);
				vertex.z = 0.0;
				polygon->SetVertex(i, vertex);
			}
			shape = polygon;
			break;
		}
		case ID_AddSphere:
		{
			auto sphere = system->Create<SphereShape>();
			sphere->SetRadius(3.0);
			shape = sphere;
			break;
		}
	}

	if (shape)
	{
		system->AddShape(shape);
		this->canvas->Refresh();
	}
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Sandbox");
	aboutDialogInfo.SetDescription("This program is design to help test the CollisionLib library.");

	wxAboutBox(aboutDialogInfo);
}