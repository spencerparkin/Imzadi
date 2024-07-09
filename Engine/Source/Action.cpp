#include "Action.h"
#include "Game.h"

using namespace Imzadi;

//------------------------------------ Action ------------------------------------

Action::Action()
{
}

/*virtual*/ Action::~Action()
{
}

/*virtual*/ void Action::Init()
{
}

/*virtual*/ void Action::Deinit()
{
}

/*virtual*/ void Action::Tick(double deltaTime)
{
}

//------------------------------------ ActionManager ------------------------------------

ActionManager::ActionManager()
{
}

/*virtual*/ ActionManager::~ActionManager()
{
}

void ActionManager::BindAction(uint32_t inputKey, Action* action)
{
	this->UnbindAction(inputKey);

	action->Init();

	this->actionMap.insert(std::pair<uint32_t, Reference<Action>>(inputKey, action));
}

void ActionManager::UnbindAction(uint32_t inputKey)
{
	ActionMap::iterator iter = this->actionMap.find(inputKey);
	if (iter != this->actionMap.end())
	{
		Action* action = iter->second;
		action->Deinit();
		this->actionMap.erase(iter);
	}
}

void ActionManager::Tick(double deltaTime)
{
	Imzadi::Controller* controller = Game::Get()->GetController(this->controllerUser);
	if (!controller)
		return;

	std::vector<uint32_t> unbindArray;

	for (auto pair : this->actionMap)
	{
		Action* action = pair.second;
		uint32_t inputKey = pair.first;

		action->Tick(deltaTime);

		if (controller->ButtonPressed(inputKey, true))
		{
			if (!action->Perform())
				unbindArray.push_back(inputKey);
		}
	}

	for (uint32_t inputKey : unbindArray)
		this->UnbindAction(inputKey);
}

void ActionManager::Clear()
{
	for (auto pair : this->actionMap)
	{
		Action* action = pair.second;
		action->Deinit();
	}

	this->actionMap.clear();
}