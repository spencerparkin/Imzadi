#pragma once

#include <wx/app.h>

class Frame;

class App : public wxApp
{
public:
	App();
	virtual ~App();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

private:
	Frame* frame;
};

wxDECLARE_APP(App);