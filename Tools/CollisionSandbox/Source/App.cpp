#include "App.h"
#include "Frame.h"
#include "Math/AxisAlignedBoundingBox.h"

wxIMPLEMENT_APP(App);

using namespace Imzadi;

#include "Math/Polygon.h"
#include <fstream>

App::App()
{
	this->frame = nullptr;
	this->collisionSystem = new Imzadi::CollisionSystem();
}

/*virtual*/ App::~App()
{
	delete this->collisionSystem;
}

/*virtual*/ bool App::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	std::vector<Imzadi::Polygon> polygonArray;

	Imzadi::Polygon polygon;
	polygon.vertexArray.push_back(Imzadi::Vector3(0.0, 0.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(2.0, 0.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(2.0, 5.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(0.0, 5.0, 0.0));
	polygonArray.push_back(polygon);

	polygon.vertexArray.clear();
	polygon.vertexArray.push_back(Imzadi::Vector3(2.0, 0.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(7.0, 0.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(7.0, 2.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(2.0, 2.0, 0.0));
	polygonArray.push_back(polygon);

	polygon.vertexArray.clear();
	polygon.vertexArray.push_back(Imzadi::Vector3(5.0, 2.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(7.0, 2.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(7.0, 5.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(5.0, 5.0, 0.0));
	polygonArray.push_back(polygon);

	polygon.vertexArray.clear();
	polygon.vertexArray.push_back(Imzadi::Vector3(0.0, 5.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(7.0, 5.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(7.0, 7.0, 0.0));
	polygon.vertexArray.push_back(Imzadi::Vector3(0.0, 7.0, 0.0));
	polygonArray.push_back(polygon);

	std::ofstream stream;
	stream.open("E:\\ENG_DEV\\Imzadi\\debug.bin", std::ios::binary);
	Imzadi::Polygon::DumpArray(polygonArray, stream);
	stream.close();

	AxisAlignedBoundingBox worldBox;
	worldBox.minCorner = Vector3(-200.0, -200.0, -200.0);
	worldBox.maxCorner = Vector3(200.0, 200.0, 200.0);

	if (!this->collisionSystem->Initialize(worldBox))
		return false;

	this->frame = new Frame(wxDefaultPosition, wxSize(1600, 1000));
	this->frame->Show();

	return true;
}

/*virtual*/ int App::OnExit(void)
{
	this->collisionSystem->Shutdown();

	return 0;
}