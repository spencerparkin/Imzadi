#include "InfoCommand.h"
#include "Game.h"
#include "Entity.h"

using namespace Imzadi;

static InfoCommand infoCommand;

InfoCommand::InfoCommand()
{
}

/*virtual*/ InfoCommand::~InfoCommand()
{
}

/*virtual*/ std::string InfoCommand::GetName()
{
	return "info";
}

/*virtual*/ std::string InfoCommand::GetSyntaxHelp()
{
	return "info [entity] <entity-name>";
}

/*virtual*/ std::string InfoCommand::GetHelpDescription()
{
	return "Inspect the state of an entity or other thing.";
}

/*virtual*/ bool InfoCommand::Execute(const std::vector<std::string>& arguments, std::vector<std::string>& results)
{
	if (arguments.size() >= 1)
	{
		if (arguments[0] == "entity" && arguments.size() == 2)
		{
			const std::string& name = arguments[1];
			Reference<Entity> foundEntity;
			if (!Game::Get()->FindEntityByName(name, foundEntity))
				results.push_back("No entity found with name: " + name);
			else
			{
				std::string info = foundEntity->GetInfo();
				results.push_back(info);
			}
		}
	}

	return true;
}