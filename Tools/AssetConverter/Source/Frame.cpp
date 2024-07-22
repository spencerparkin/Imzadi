#include "Frame.h"
#include "Canvas.h"
#include "App.h"
#include "GamePreview.h"
#include "Converter.h"
#include "Log.h"
#include "LogWindow.h"
#include "RenderObjectList.h"
#include "RenderObjectProperties.h"
#include "FontMaker.h"
#include "RenderObjects/TextRenderObject.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/choicdlg.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/filename.h>

Frame::Frame(const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY, "Imzadi Asset Converter", pos, size), timer(this, ID_Timer)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ConvertAsset, "Convert Asset...", "Convert the output of some artsy-fartsy 3D modeling program into assets consumable as input to the Imzadi Game Engine."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_PreviewAsset, "Preview Asset...", "Select one or more Imzadi asset files to load and preview in engine."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ClearScene, "Clear Scene", "Remove all render objects from the current scene."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Go skiing."));

	wxMenu* optionsMenu = new wxMenu();
	optionsMenu->Append(new wxMenuItem(optionsMenu, ID_ShowLogWindow, "Show Log Window", "Remove or bring back a window showing logging output.", wxITEM_CHECK));
	optionsMenu->Append(new wxMenuItem(optionsMenu, ID_ShowSkeleton, "Show Skeleton", "Render line-segments to illustrate the skeleton of skinned meshes being previews.", wxITEM_CHECK));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(optionsMenu, "Options");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->CreateStatusBar();

	this->Bind(wxEVT_MENU, &Frame::OnConvertAsset, this, ID_ConvertAsset);
	this->Bind(wxEVT_MENU, &Frame::OnPreviewAsset, this, ID_PreviewAsset);
	this->Bind(wxEVT_MENU, &Frame::OnClearScene, this, ID_ClearScene);
	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnShowLogWindow, this, ID_ShowLogWindow);
	this->Bind(wxEVT_MENU, &Frame::OnShowSkeleton, this, ID_ShowSkeleton);
	this->Bind(wxEVT_TIMER, &Frame::OnTimer, this, ID_Timer);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ShowLogWindow);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ShowSkeleton);
	this->Bind(wxEVT_CLOSE_WINDOW, &Frame::OnCloseWindow, this);

	wxSplitterWindow* mainSplitter = new wxSplitterWindow(this, wxID_ANY);
	mainSplitter->SetMinimumPaneSize(100);

	this->canvas = new Canvas(mainSplitter);

	wxPanel* sidePanel = new wxPanel(mainSplitter);
	wxSplitterWindow* sideSplitter = new wxSplitterWindow(sidePanel, wxID_ANY);

	this->renderObjectList = new RenderObjectList(sideSplitter);
	this->renderObjectProperties = new RenderObjectProperties(sideSplitter);

	wxBoxSizer* sideSizer = new wxBoxSizer(wxVERTICAL);
	sideSizer->Add(sideSplitter, 1, wxALL | wxEXPAND, 0);
	sidePanel->SetSizer(sideSizer);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(mainSplitter, 1, wxALL | wxEXPAND, 0);
	this->SetSizer(mainSizer);

	mainSplitter->SplitVertically(this->canvas, sidePanel);
	mainSplitter->SetSashPosition(800);

	sideSplitter->SplitHorizontally(this->renderObjectList, this->renderObjectProperties);
	sideSplitter->SetSashPosition(200);

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

	Imzadi::Game::Get()->Run();

	this->inTimer = false;
}

void Frame::OnShowLogWindow(wxCommandEvent& event)
{
	if (!Imzadi::LoggingSystem::Get()->RouteExists("log_window"))
		Imzadi::LoggingSystem::Get()->AddRoute(new LogWindowRoute());
	else
		Imzadi::LoggingSystem::Get()->RemoveRoute("log_window");
}

void Frame::OnShowSkeleton(wxCommandEvent& event)
{
	Imzadi::AnimatedMeshInstance::SetRenderSkeletons(!Imzadi::AnimatedMeshInstance::GetRenderSkeletons());
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_ShowLogWindow:
		{
			event.Check(Imzadi::LoggingSystem::Get()->RouteExists("log_window"));
			break;
		}
		case ID_ShowSkeleton:
		{
			event.Check(Imzadi::AnimatedMeshInstance::GetRenderSkeletons());
			break;
		}
	}
}

void Frame::OnConvertAsset(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, "Choose file(s) to convert.  "
		"Note that for dynamic meshes, they must be exported in bind-pose.  "
		"For animations, they must be exported with the animation applied.",
		wxEmptyString, wxEmptyString, "Any file (*.*)|*.*", wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
	if (fileDialog.ShowModal() != wxID_OK)
		return;

	Converter converter;
	FontMaker fontMaker;

	uint32_t textureMakerFlags = 0;
	uint32_t converterFlags = 0;

	bool textureMakerFlagsNeeded = false;
	bool converterFlagsNeeded = false;

	wxArrayString fileArray;
	fileDialog.GetPaths(fileArray);
	for (const wxString& file : fileArray)
	{
		wxFileName fileName(file);
		wxString ext = fileName.GetExt().Lower();

		if (ext == "png")
		{
			textureMakerFlagsNeeded = true;
		}
		else if (ext == "fbx")
		{
			textureMakerFlagsNeeded = true;
			converterFlagsNeeded = true;
		}
	}

	if (converterFlagsNeeded)
	{
		std::vector<FlagChoice> flagChoiceArray = {
			{"Meshes", Converter::Flag::CONVERT_MESHES},
			{"Animations", Converter::Flag::CONVERT_ANIMATIONS},
			{"Collision", Converter::Flag::MAKE_COLLISION},
			{"Sky Dome", Converter::Flag::CONVERT_SKYDOME},
			{"Center Obj. Space at Origin", Converter::Flag::CENTER_OBJ_SPACE_AT_ORIGIN}
		};

		if (!this->FlagsFromDialog("Import what (and how) across all chosen export files?", flagChoiceArray, converterFlags))
			return;
	}

	if (textureMakerFlagsNeeded)
	{
		std::vector<FlagChoice> flagChoiceArray = {
			{"Color", TextureMaker::Flag::COLOR},
			{"Alpha", TextureMaker::Flag::ALPHA},
			{"Compress", TextureMaker::Flag::COMPRESS},
			{"Flip Vertical", TextureMaker::Flag::FLIP_VERTICAL},
			{"Flip Horizontal", TextureMaker::Flag::FLIP_HORIZONTAL},
			{"Make Alpha", TextureMaker::Flag::MAKE_ALPHA},
			{"Make Mips", TextureMaker::Flag::MIP_MAPS},
			{"For Cube-Map", TextureMaker::Flag::FOR_CUBE_MAP}
		};

		if (!this->FlagsFromDialog("Build textures how across all selected textures or those found in all chosen export files?", flagChoiceArray, textureMakerFlags))
			return;
	}

	for (const wxString& file : fileArray)
	{
		wxFileName fileName(file);
		wxString ext = fileName.GetExt().Lower();

		if (ext == "ttf")
		{
			fontMaker.MakeFont(file);
		}
		else if (ext == "png")
		{
			TextureMaker textureMaker;
			textureMaker.MakeTexture(file, textureMakerFlags);
		}
		else
		{
			converter.SetFlags(converterFlags);
			converter.SetTextureMakerFlags(textureMakerFlags);
			converter.Convert(file);
		}
	}
}

bool Frame::FlagsFromDialog(const wxString& prompt, const std::vector<FlagChoice>& flagChoiceArray, uint32_t& chosenFlags)
{
	chosenFlags = 0;

	wxArrayString choiceArray;
	for (const FlagChoice& flagChoice : flagChoiceArray)
		choiceArray.Add(flagChoice.name);

	wxMultiChoiceDialog choiceDialog(this, prompt, "Choose, Please", choiceArray);
	if (choiceDialog.ShowModal() != wxID_OK)
		return false;

	wxArrayInt selectionArray = choiceDialog.GetSelections();
	for (int i = 0; i < selectionArray.size(); i++)
	{
		const wxString& selection = choiceArray[selectionArray[i]];
		for (const FlagChoice& flagChoice : flagChoiceArray)
		{
			if (flagChoice.name == selection)
			{
				chosenFlags |= flagChoice.flag;
				break;
			}
		}
	}

	return true;
}

void Frame::OnPreviewAsset(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, "Choose file(s) to preview.", wxEmptyString, wxEmptyString, "Any file (*.*)|*.*", wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
	if (fileDialog.ShowModal() != wxID_OK)
		return;

	Imzadi::Game* game = Imzadi::Game::Get();
	wxString errorMsg;

	wxArrayString fileArray;
	fileDialog.GetPaths(fileArray);
	for (const wxString& file : fileArray)
	{
		wxFileName fileName(file);
		wxString ext = fileName.GetExt().Lower();

		Imzadi::Reference<Imzadi::RenderObject> renderObject;

		if (ext == "font")
		{
			Imzadi::Reference<Imzadi::Asset> asset;
			if (game->GetAssetCache()->LoadAsset((const char*)file, asset))
			{
				Imzadi::Transform rotation;
				rotation.SetIdentity();
				rotation.matrix.SetFromAxisAngle(Imzadi::Vector3(0.0, 1.0, 0.0), 0.0);
				Imzadi::Transform scale;
				scale.SetIdentity();
				scale.matrix.SetUniformScale(10.0);
				Imzadi::Transform translation;
				translation.SetIdentity();
				translation.translation.SetComponents(0.0, 0.0, -10.0);
				auto textRenderObject = new Imzadi::TextRenderObject();
				textRenderObject->SetText("The quick brown fox jumped over the lazy dog.");
				textRenderObject->SetFont((const char*)fileName.GetName());
				textRenderObject->SetForegroundColor(Imzadi::Vector3(1.0, 1.0, 1.0));
				textRenderObject->SetTransform(translation * rotation * scale);
				textRenderObject->SetFlags(Imzadi::TextRenderObject::Flag::CENTER_JUSTIFY | Imzadi::TextRenderObject::Flag::STICK_WITH_CAMERA);
				renderObject.Set(textRenderObject);
				game->GetScene()->AddRenderObject(textRenderObject);
			}
		}
		else
		{
			renderObject = game->LoadAndPlaceRenderMesh((const char*)file.c_str());
		}

		if (!renderObject)
			errorMsg += wxString::Format("Failed to make render object for asset: %s\n", file.c_str());
		else
			this->renderObjectList->AddRenderObject(renderObject.Get());
	}

	if (errorMsg.length() > 0)
		wxMessageBox(errorMsg, "Error!", wxICON_ERROR | wxOK, this);

	this->renderObjectList->UpdateListView();
}

void Frame::OnClearScene(wxCommandEvent& event)
{
	this->renderObjectList->Clear();
	this->renderObjectList->UpdateListView();
	this->renderObjectProperties->Clear();

	Imzadi::Game* game = Imzadi::Game::Get();
	game->GetScene()->Clear();
	game->GetScene()->AddRenderObject(game->GetDebugLines());
	game->GetAssetCache()->Clear();
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Imzadi Asset Converter");
	aboutDialogInfo.SetDescription("This program is designed to convert art program files into assets consumable by the Imzadi Game Engine.  It also provides a preview of the converted assets.");

	wxAboutBox(aboutDialogInfo);
}

void Frame::OnCloseWindow(wxCloseEvent& event)
{
	this->renderObjectList->Clear();

	wxFrame::OnCloseWindow(event);
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close();
}