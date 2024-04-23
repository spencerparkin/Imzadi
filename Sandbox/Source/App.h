#pragma once

#include <wx/app.h>
#include "System.h"

class Frame;

class App : public wxApp
{
public:
	App();
	virtual ~App();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Collision::System* GetCollisionSystem() { return this->collisionSystem; }

private:
	Frame* frame;
	Collision::System* collisionSystem;
};

wxDECLARE_APP(App);