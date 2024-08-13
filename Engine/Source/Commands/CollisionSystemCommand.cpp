#include "CollisionSystemCommand.h"

using namespace Imzadi;

static CollisionSystemCommand collisionSystemCommand;

CollisionSystemCommand::CollisionSystemCommand()
{
}

/*virtual*/ CollisionSystemCommand::~CollisionSystemCommand()
{
}

/*virtual*/ std::string CollisionSystemCommand::GetName()
{
	return "collsys";
}

/*virtual*/ std::string CollisionSystemCommand::GetSyntaxHelp()
{
	return "collsys ...";
}

/*virtual*/ std::string CollisionSystemCommand::GetHelpDescription()
{
	return "Inspect or tweak the collision system internals.";
}

/*virtual*/ void CollisionSystemCommand::Execute(const std::vector<std::string>& arguments)
{
	//...
}