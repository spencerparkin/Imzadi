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

void ActionManager::BindAction(Button inputKey, Action* action)
{
	this->UnbindAction(inputKey);

	action->Init();

	this->actionMap.insert(std::pair<Button, Reference<Action>>(inputKey, action));
}

void ActionManager::UnbindAction(Button inputKey)
{
	ActionMap::iterator iter = this->actionMap.find(inputKey);
	if (iter != this->actionMap.end())
	{
		Action* action = iter->second;
		action->Deinit();
		this->actionMap.erase(iter);
	}
}

bool ActionManager::IsBound(Button inputKey)
{
	ActionMap::iterator iter = this->actionMap.find(inputKey);
	return iter != this->actionMap.end();
}

Action* ActionManager::GetBoundAction(Button inputKey)
{
	ActionMap::iterator iter = this->actionMap.find(inputKey);
	if (iter == this->actionMap.end())
		return nullptr;

	return iter->second;
}

void ActionManager::Tick(double deltaTime)
{
	Imzadi::Input* controller = Game::Get()->GetController(this->controllerUser);
	if (!controller)
		return;

	std::vector<Button> unbindArray;

	for (auto pair : this->actionMap)
	{
		Action* action = pair.second;
		Button inputKey = pair.first;

		action->Tick(deltaTime);

		if (controller->ButtonPressed(inputKey, true))
		{
			if (!action->Perform())
				unbindArray.push_back(inputKey);
		}
	}

	for (Button inputKey : unbindArray)
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