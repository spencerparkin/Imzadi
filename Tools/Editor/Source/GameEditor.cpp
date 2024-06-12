#include "GameEditor.h"
#include "App.h"
#include "Frame.h"
#include "Canvas.h"
#include "EditorAssetCache.h"

GameEditor::GameEditor(HINSTANCE instance) : Game(instance)
{
}

/*virtual*/ GameEditor::~GameEditor()
{
}

/*virtual*/ bool GameEditor::PostInit()
{
	this->SetAssetCache(new EditorAssetCache());

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

bool GameEditor::Import(const aiScene* scene, wxString& error)
{
	auto editorAssetCache = dynamic_cast<EditorAssetCache*>(this->GetAssetCache());
	if (!editorAssetCache)
		return false;

	editorAssetCache->BeginImport(scene);

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		if (!this->LoadAndPlaceRenderMesh("Mesh:" + std::string(mesh->mName.C_Str()), Imzadi::Vector3(), Imzadi::Quaternion()))
			error += wxString::Format("Failed to load mesh asset: %s", mesh->mName.C_Str());
	}

	editorAssetCache->EndImport();

	return error.Length() == 0;
}