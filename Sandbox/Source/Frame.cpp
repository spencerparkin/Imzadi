#include "Frame.h"
#include "App.h"
#include "Canvas.h"
#include "Error.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "Shapes/Box.h"
#include "Shapes/Capsule.h"
#include "Shapes/Polygon.h"
#include "Shapes/Sphere.h"
#include "Command.h"

using namespace Collision;

Frame::Frame(const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY, "Sandbox", pos, size), timer(this, ID_Timer)
{
	this->canvas = nullptr;

	wxMenu* programMenu = new wxMenu();
	programMenu->Append(new wxMenuItem(programMenu, ID_DumpWorld, "Dump World", "Write the current physics world to disk."));
	programMenu->Append(new wxMenuItem(programMenu, ID_RestoreWorld, "Restore World", "Load the physics world from disk."));
	programMenu->AppendSeparator();
	programMenu->Append(new wxMenuItem(programMenu, ID_ClearWorld, "Clear World", "Remove all shapes from the collision world."));
	programMenu->AppendSeparator();
	programMenu->Append(new wxMenuItem(programMenu, ID_Exit, "Exit", "Go skiing."));

	wxMenu* shapeMenu = new wxMenu();
	shapeMenu->Append(new wxMenuItem(shapeMenu, ID_AddBox, "Add Box", "Add a box shape to the collision world."));
	shapeMenu->Append(new wxMenuItem(shapeMenu, ID_AddCapsule, "Add Capsule", "Add a capsule shape to the collision world."));
	shapeMenu->Append(new wxMenuItem(shapeMenu, ID_AddPolygon, "Add Polygon", "Add a polygon shape to the collision world."));
	shapeMenu->Append(new wxMenuItem(shapeMenu, ID_AddSphere, "Add Sphere", "Add a sphere shape to the collision world."));

	wxMenu* drawMenu = new wxMenu();
	drawMenu->Append(new wxMenuItem(drawMenu, ID_DrawShapes, "Draw Shapes", "When checked, draw the collision shapes in the collision world.", wxITEM_CHECK));
	drawMenu->Append(new wxMenuItem(drawMenu, ID_DrawShapeBoxes, "Draw Shape Boxes", "When checked, draw the bounding boxes of the collision shapes.", wxITEM_CHECK));
	drawMenu->Append(new wxMenuItem(drawMenu, ID_DrawBoxTree, "Draw Box Tree", "When checked, draw the bounding box tree used to partition space in the collision world.", wxITEM_CHECK));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(programMenu, "Program");
	menuBar->Append(shapeMenu, "Shape");
	menuBar->Append(drawMenu, "Draw");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));

	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnClearWorld, this, ID_ClearWorld);
	this->Bind(wxEVT_MENU, &Frame::OnDumpOrRestoreWorld, this, ID_DumpWorld);
	this->Bind(wxEVT_MENU, &Frame::OnDumpOrRestoreWorld, this, ID_RestoreWorld);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnAddShape, this, ID_AddBox);
	this->Bind(wxEVT_MENU, &Frame::OnAddShape, this, ID_AddCapsule);
	this->Bind(wxEVT_MENU, &Frame::OnAddShape, this, ID_AddPolygon);
	this->Bind(wxEVT_MENU, &Frame::OnAddShape, this, ID_AddSphere);
	this->Bind(wxEVT_MENU, &Frame::OnDebugDrawToggle, this, ID_DrawShapes);
	this->Bind(wxEVT_MENU, &Frame::OnDebugDrawToggle, this, ID_DrawShapeBoxes);
	this->Bind(wxEVT_MENU, &Frame::OnDebugDrawToggle, this, ID_DrawBoxTree);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_DrawShapes);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_DrawShapeBoxes);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_DrawBoxTree);
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

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
	case ID_DrawShapes:
		event.Check((this->canvas->GetDebugDrawFlags() & COLL_SYS_DRAW_FLAG_SHAPES) != 0);
		break;
	case ID_DrawShapeBoxes:
		event.Check((this->canvas->GetDebugDrawFlags() & COLL_SYS_DRAW_FLAG_SHAPE_BOXES) != 0);
		break;
	case ID_DrawBoxTree:
		event.Check((this->canvas->GetDebugDrawFlags() & COLL_SYS_DRAW_FLAG_AABB_TREE) != 0);
		break;
	}
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
		wxString statusText = wxString::Format("Frame-rate: %02.2f FPS", frameRateFPS);
		statusText += " | " + this->canvas->GetStatusMessage();
		this->GetStatusBar()->SetStatusText(statusText);
	}

	this->inTimer = false;
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnClearWorld(wxCommandEvent& event)
{
	System* system = wxGetApp().GetCollisionSystem();
	system->IssueCommand(system->Create<RemoveAllShapesCommand>());
	this->canvas->Refresh();
}

void Frame::OnDumpOrRestoreWorld(wxCommandEvent& event)
{
	System* system = wxGetApp().GetCollisionSystem();

	switch (event.GetId())
	{
		case ID_DumpWorld:
		{
			wxFileDialog fileDialog(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "(*.bin)|*.bin", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (fileDialog.ShowModal() == wxID_OK)
			{
				std::string fileName((const char*)fileDialog.GetPath().c_str());

				if (wxFileExists(fileName))
					wxRemoveFile(fileName);

				if (!system->DumpToFile(fileName))
				{
					wxString errorMsg(GetError()->GetAllErrorMessages().c_str());
					wxMessageBox(errorMsg, "Error!", wxICON_ERROR | wxOK, this);
				}
				else
				{
					wxMessageBox("Physics world dumped!", "Success!", wxICON_INFORMATION | wxOK, this);
				}
			}

			break;
		}
		case ID_RestoreWorld:
		{
			wxFileDialog fileDialog(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "(*.bin)|*.bin", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (fileDialog.ShowModal() == wxID_OK)
			{
				std::string fileName((const char*)fileDialog.GetPath().c_str());
				if (!system->RestoreFromFile(fileName))
				{
					wxString errorMsg(GetError()->GetAllErrorMessages().c_str());
					wxMessageBox(errorMsg, "Error!", wxICON_ERROR | wxOK, this);
				}
				else
				{
					wxMessageBox("Physics world restored!", "Success!", wxICON_INFORMATION | wxOK, this);
				}
			}

			break;
		}
	}
}

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
			Transform objectToWorld;
			objectToWorld.SetIdentity();
			objectToWorld.matrix.SetFromAxisAngle(Vector3(1.0, 1.0, 1.0).Normalized(), M_PI / 3.0);
			objectToWorld.translation.SetComponents(13.0, 5.0, -16.0);
			box->SetExtents(Vector3(3.0, 4.0, 5.0));
			box->SetObjectToWorldTransform(objectToWorld);
			shape = box;
			break;
		}
		case ID_AddCapsule:
		{
			auto capsule = system->Create<CapsuleShape>();
			capsule->SetRadius(2.0);
			capsule->SetVertex(0, Vector3(-5.0, 0.0, 0.0));
			capsule->SetVertex(1, Vector3(5.0, 0.0, 0.0));
			shape = capsule;
			break;
		}
		case ID_AddPolygon:
		{
			auto polygon = system->Create<PolygonShape>();
			Transform objectToWorld;
			objectToWorld.SetIdentity();
			objectToWorld.translation.SetComponents(-20.0, -20.0, -20.0);
			objectToWorld.matrix.SetFromAxisAngle(Vector3(1.0, 1.0, 2.0).Normalized(), M_PI / 4.0);
			polygon->SetObjectToWorldTransform(objectToWorld);
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
			Transform objectToWorld;
			objectToWorld.SetIdentity();
			objectToWorld.translation.SetComponents(-20.0, 20.0, -20.0);
			objectToWorld.matrix.SetFromAxisAngle(Vector3(1.0, 1.0, 2.0).Normalized(), M_PI / 4.0);
			sphere->SetObjectToWorldTransform(objectToWorld);
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

void Frame::OnDebugDrawToggle(wxCommandEvent& event)
{
	uint32_t toggleFlag = 0;
	
	switch (event.GetId())
	{
	case ID_DrawShapes:
		toggleFlag = COLL_SYS_DRAW_FLAG_SHAPES;
		break;
	case ID_DrawShapeBoxes:
		toggleFlag = COLL_SYS_DRAW_FLAG_SHAPE_BOXES;
		break;
	case ID_DrawBoxTree:
		toggleFlag = COLL_SYS_DRAW_FLAG_AABB_TREE;
		break;
	}

	uint32_t drawFlags = this->canvas->GetDebugDrawFlags();
	drawFlags ^= toggleFlag;
	this->canvas->SetDebugDrawFlags(drawFlags);
	this->canvas->Refresh();
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Sandbox");
	aboutDialogInfo.SetDescription("This program is design to help test and develop the CollisionLib library.");

	wxAboutBox(aboutDialogInfo);
}