#include "GamePreview.h"
#include "App.h"
#include "Frame.h"
#include "Canvas.h"
#include "Entities/FreeCam.h"
#include <wx/msgdlg.h>

GamePreview::GamePreview(HINSTANCE instance) : Game(instance)
{
}

/*virtual*/ GamePreview::~GamePreview()
{
}

/*virtual*/ bool GamePreview::PostInit()
{
	Imzadi::Camera* camera = Game::Get()->GetCamera();
	camera->LookAt(Imzadi::Vector3(0.0, 0.0, -10.0), Imzadi::Vector3(0.0, 0.0, 0.0), Imzadi::Vector3(0.0, 1.0, 0.0));

	auto freeCam = this->SpawnEntity<Imzadi::FreeCam>();
	freeCam->SetEnabled(true);
	freeCam->SetCamera(camera);

	return Game::PostInit();
}

/*virtual*/ bool GamePreview::CreateRenderWindow()
{
	// Have the Imzadi engine use our window rather than create its own window.
	Canvas* canvas = wxGetApp().GetFrame()->GetCanvas();
	this->SetMainWindowHandle(canvas->GetHWND());
	return true;
}

/*virtual*/ void GamePreview::PumpWindowsMessages()
{
	// Do nothing here to prevent the engine from pumping windows messages.
	// The wxWidgets windowing framework will pump messages in its own way.
}

/*virtual*/ bool GamePreview::PreShutdown()
{
	this->animatedMesh.Reset();

	return Game::PreShutdown();
}

/*virtual*/ void GamePreview::Tick(Imzadi::TickPass tickPass)
{
	Game::Tick(tickPass);

	if (tickPass == Imzadi::TickPass::PARALLEL_WORK)
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

		if (this->animatedMesh)
			this->animatedMesh->AdvanceAnimation(this->GetDeltaTime());
	}
}

void GamePreview::SetAnimatingMesh(Imzadi::AnimatedMeshInstance* instance)
{
	this->animatedMesh.Set(instance);
}