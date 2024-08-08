#pragma once

#include <wx/app.h>
#include "Collision/System.h"
#include "Math/Polygon.h"

class Frame;

class App : public wxApp
{
public:
	App();
	virtual ~App();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Imzadi::CollisionSystem* GetCollisionSystem() { return this->collisionSystem; }
	std::vector<Imzadi::Polygon>& GetPolygonArray() { return this->polygonArray; }

private:
	Frame* frame;
	Imzadi::CollisionSystem* collisionSystem;
	std::vector<Imzadi::Polygon> polygonArray;
};

wxDECLARE_APP(App);