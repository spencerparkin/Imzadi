#include "RubiksCubie.h"

RubiksCubie::RubiksCubie()
{
}

/*virtual*/ RubiksCubie::~RubiksCubie()
{
}

/*virtual*/ bool RubiksCubie::Setup()
{
	return true;
}

/*virtual*/ bool RubiksCubie::Shutdown()
{
	return true;
}

/*virtual*/ bool RubiksCubie::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	return true;
}