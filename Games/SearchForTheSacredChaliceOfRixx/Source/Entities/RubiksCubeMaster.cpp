#include "RubiksCubeMaster.h"

RubiksCubeMaster::RubiksCubeMaster()
{
}

/*virtual*/ RubiksCubeMaster::~RubiksCubeMaster()
{
}

/*virtual*/ bool RubiksCubeMaster::Setup()
{
	return true;
}

/*virtual*/ bool RubiksCubeMaster::Shutdown()
{
	return true;
}

/*virtual*/ bool RubiksCubeMaster::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	return true;
}