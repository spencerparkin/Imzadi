#pragma once

#include <wx/app.h>
#include "Collision/System.h"

class Frame;

class App : public wxApp
{
public:
	App();
	virtual ~App();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Imzadi::CollisionSystem* GetCollisionSystem() { return this->collisionSystem; }

private:
	Frame* frame;
	Imzadi::CollisionSystem* collisionSystem;
};

wxDECLARE_APP(App);