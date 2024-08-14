#include "CollisionSystemCommand.h"
#include "Collision/Command.h"
#include "Collision/Query.h"
#include "Collision/Result.h"
#include "Game.h"
#include <filesystem>

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
	return "collsys [stats|dump]";
}

/*virtual*/ std::string CollisionSystemCommand::GetHelpDescription()
{
	return "Inspect or tweak the collision system internals.";
}

/*virtual*/ bool CollisionSystemCommand::Execute(const std::vector<std::string>& arguments, std::vector<std::string>& results)
{
	if (arguments.size() < 1)
		return false;

	Collision::System* collisionSystem = Game::Get()->GetCollisionSystem();

	if (arguments[0] == "stats")
	{
		auto query = new Collision::StatsQuery();
		Collision::TaskID taskID = 0;
		collisionSystem->MakeQuery(query, taskID);
		collisionSystem->FlushAllTasks();
		Collision::Result* result = collisionSystem->ObtainQueryResult(taskID);
		if (result)
		{
			auto statsResult = dynamic_cast<Collision::StatsResult*>(result);
			if (statsResult)
			{
				results.push_back(std::format("Num. shapes: {}", statsResult->numShapes));
				for (auto pair : statsResult->shapeCountMap)
					results.push_back(std::format("Num. {} shapes: {}", Collision::Shape::ShapeTypeLabel(pair.first).c_str(), pair.second));
			}
			delete result;
		}
	}
	else if (arguments[0] == "dump" && arguments.size() == 2)
	{
		std::string filePath = arguments[1];
		auto command = new Collision::FileCommand();
		command->SetFilePath(filePath);
		command->SetAction(Collision::FileCommand::DUMP);
		collisionSystem->IssueCommand(command);
		collisionSystem->FlushAllTasks();
		if (std::filesystem::exists(filePath))
			results.push_back("Dumped: " + filePath);
		else
			results.push_back("Dump failed.");
	}

	return true;
}