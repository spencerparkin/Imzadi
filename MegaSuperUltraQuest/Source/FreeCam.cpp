#include "FreeCam.h"

FreeCam::FreeCam()
{
	this->enabled = false;
}

/*virtual*/ FreeCam::~FreeCam()
{
}

/*virtual*/ bool FreeCam::Setup()
{
	if (!this->camera)
		return false;

	return true;
}

/*virtual*/ bool FreeCam::Shutdown(bool gameShuttingDown)
{
	return true;
}

/*virtual*/ void FreeCam::Tick(double deltaTime)
{
	if (!this->enabled)
		return;

	// TODO: Use the controller to move the camera around.
}