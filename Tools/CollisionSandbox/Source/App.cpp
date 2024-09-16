#include "App.h"
#include "Frame.h"
#include "Canvas.h"
#include "Math/AxisAlignedBoundingBox.h"

wxIMPLEMENT_APP(App);

using namespace Imzadi;

App::App()
{
	this->frame = nullptr;
	this->collisionSystem = new Imzadi::Collision::System();
}

/*virtual*/ App::~App()
{
	delete this->collisionSystem;
}

/*virtual*/ bool App::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	AxisAlignedBoundingBox worldBox;
	worldBox.minCorner = Vector3(-200.0, -200.0, -200.0);
	worldBox.maxCorner = Vector3(200.0, 200.0, 200.0);

	if (!this->collisionSystem->Initialize(worldBox))
		return false;

	this->frame = new Frame(wxDefaultPosition, wxSize(1600, 1000));
	this->frame->Show();
	this->frame->GetCanvas()->SetupInput();

	return true;
}

/*virtual*/ int App::OnExit(void)
{
	this->collisionSystem->Shutdown();

	return 0;
}