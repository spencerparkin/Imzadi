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

/*virtual*/ bool CollisionSystemCommand::Execute(const std::vector<std::string>& arguments, std::vector<std::string>& results)
{
	if (arguments.size() < 1)
		return false;

	if (arguments[0] == "stats")
	{
		// TODO: Issue stats query here.
	}

	return true;
}