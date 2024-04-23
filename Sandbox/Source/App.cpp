#include "App.h"
#include "Frame.h"

wxIMPLEMENT_APP(App);

App::App()
{
	this->frame = nullptr;
}

/*virtual*/ App::~App()
{
}

/*virtual*/ bool App::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new Frame(wxDefaultPosition, wxDefaultSize);
	this->frame->Show();

	return true;
}

/*virtual*/ int App::OnExit(void)
{
	return 0;
}