#include "GameEditor.h"
#include "App.h"
#include "Frame.h"
#include "Canvas.h"
#include "EditorAssetCache.h"
#include "Entities/FreeCam.h"
#include <wx/msgdlg.h>

GameEditor::GameEditor(HINSTANCE instance) : Game(instance)
{
}

/*virtual*/ GameEditor::~GameEditor()
{
}

/*virtual*/ bool GameEditor::PostInit()
{
	this->SetAssetCache(new EditorAssetCache());

	Imzadi::Camera* camera = Game::Get()->GetCamera();
	camera->LookAt(Imzadi::Vector3(0.0, 0.0, -10.0), Imzadi::Vector3(0.0, 0.0, 0.0), Imzadi::Vector3(0.0, 1.0, 0.0));

	auto freeCam = this->SpawnEntity<Imzadi::FreeCam>();
	freeCam->SetEnabled(true);
	freeCam->SetCamera(camera);

	return Game::PostInit();
}

/*virtual*/ bool GameEditor::CreateRenderWindow()
{
	// Have the Imzadi engine use our window rather than create its own window.
	Canvas* canvas = wxGetApp().GetFrame()->GetCanvas();
	this->SetMainWindowHandle(canvas->GetHWND());
	return true;
}

/*virtual*/ void GameEditor::PumpWindowsMessages()
{
	// Do nothing here to prevent the engine from pumping windows messages.
	// wxWidgets will pump messages in its own way.
}

/*virtual*/ void GameEditor::Tick(Imzadi::TickPass tickPass, double deltaTimeSeconds)
{
	Game::Tick(tickPass, deltaTimeSeconds);

	if (tickPass == Imzadi::TickPass::MID_TICK)
	{
		Imzadi::DebugLines* debugLines = Game::Get()->GetDebugLines();

		Imzadi::DebugLines::Line xAxis;
		xAxis.color.SetComponents(1.0, 0.0, 0.0);
		xAxis.segment.point[0].SetComponents(0.0, 0.0, 0.0);
		xAxis.segment.point[1].SetComponents(1.0, 0.0, 0.0);
		debugLines->AddLine(xAxis);

		Imzadi::DebugLines::Line yAxis;
		yAxis.color.SetComponents(0.0, 1.0, 0.0);
		yAxis.segment.point[0].SetComponents(0.0, 0.0, 0.0);
		yAxis.segment.point[1].SetComponents(0.0, 1.0, 0.0);
		debugLines->AddLine(yAxis);

		Imzadi::DebugLines::Line zAxis;
		zAxis.color.SetComponents(0.0, 0.0, 1.0);
		zAxis.segment.point[0].SetComponents(0.0, 0.0, 0.0);
		zAxis.segment.point[1].SetComponents(0.0, 0.0, 1.0);
		debugLines->AddLine(zAxis);
	}
}

bool GameEditor::Import(const aiScene* scene, wxString& error)
{
	auto editorAssetCache = dynamic_cast<EditorAssetCache*>(this->GetAssetCache());
	if (!editorAssetCache)
		return false;

	bool rigAndAnimate = false;
	int answer = wxMessageBox("Do you plan to rig and animate the model?", "Static vs. Dynamic Mesh", wxICON_QUESTION | wxYES_NO, wxGetApp().GetFrame());
	if (answer == wxYES)
		rigAndAnimate = true;

	editorAssetCache->BeginImport(scene, rigAndAnimate);

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		if (!this->LoadAndPlaceRenderMesh("Mesh:" + std::string(mesh->mName.C_Str()), Imzadi::Vector3(), Imzadi::Quaternion()))
			error += wxString::Format("Failed to load mesh asset: %s", mesh->mName.C_Str());
	}

	editorAssetCache->EndImport();

	return error.Length() == 0;
}