#include "Frame.h"
#include "Canvas.h"
#include "App.h"
#include "GameEditor.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/config.h"

Frame::Frame(const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY, "Imzadi Game Editor", pos, size), timer(this, ID_Timer)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Import, "Import", "Import a 3D model from some artsy-fartsy program like Blender of 3Ds Max."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Export, "Export", "Export a 3D model to a form that the Imzadi engine can consume for run-time purposes."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Go skiing."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->CreateStatusBar();

	this->Bind(wxEVT_MENU, &Frame::OnImport, this, ID_Import);
	this->Bind(wxEVT_MENU, &Frame::OnExport, this, ID_Export);
	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_TIMER, &Frame::OnTimer, this, ID_Timer);

	this->canvas = new Canvas(this);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizer->Add(this->canvas, 1, wxALL | wxEXPAND, 0);
	this->SetSizer(boxSizer);

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

	wxGetApp().GetGameEditor()->Run();

	this->inTimer = false;
}

void Frame::OnImport(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, "Choose file(s) to import.", wxEmptyString, wxEmptyString, "Any file (*.*)|*.*", wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
	if (fileDialog.ShowModal() == wxID_OK)
	{
		GameEditor* gameEditor = wxGetApp().GetGameEditor();
		Assimp::Importer importer;
		wxString errorMsg;

		importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0);

		wxArrayString fileArray;
		fileDialog.GetPaths(fileArray);
		for (const wxString& file : fileArray)
		{
			wxFileName fileName(file);
			std::string fileFolder((const char*)fileName.GetPath().c_str());
			gameEditor->GetAssetCache()->AddAssetFolder(fileFolder);

			const aiScene* scene = importer.ReadFile(file.c_str(), aiProcess_PreTransformVertices | aiProcess_Triangulate | aiProcess_GlobalScale);
			if (!scene)
				errorMsg += wxString::Format("%s:\nImport error: %s\n", file.c_str(), importer.GetErrorString());
			else
			{
				wxString importError;
				if (!gameEditor->Import(scene, importError))
					errorMsg += importError + "\n";
			}

			gameEditor->GetAssetCache()->RemoveAssetFolder(fileFolder);
		}

		if (errorMsg.length() > 0)
			wxMessageBox(errorMsg, "Error!", wxICON_ERROR | wxOK, this);
	}
}

void Frame::OnExport(wxCommandEvent& event)
{
	// TODO: Have user select folder where assets would go, and then enter a base name for the assets, I suppose.
	//       Then walk through generated assets in our cache, dumping them to disk.
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Imzadi Game Editor");
	aboutDialogInfo.SetDescription("This program is designed to convert art program files into assets consumable by the Imzadi Game Engine.");

	wxAboutBox(aboutDialogInfo);
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close();
}