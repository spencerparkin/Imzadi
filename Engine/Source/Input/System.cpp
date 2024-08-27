#include "Input/System.h"
#include "Input.h"
#include "PCInput.h"
#include "XInput.h"
#include "DInput.h"

using namespace Imzadi;

InputSystem::InputSystem()
{
}

/*virtual*/ InputSystem::~InputSystem()
{
}

bool InputSystem::Setup(HWND windowHandle)
{
	int playerNumber = 0;
	while (true)
	{
		Player* player = this->MakePlayer<DInput>(playerNumber, windowHandle);
		if (!player)
		{
			player = this->MakePlayer<XInput>(playerNumber, windowHandle);
			if (!player)
			{
				player = this->MakePlayer<PCInput>(playerNumber, windowHandle);
				if (!player)
					break;
			}
		}

		this->playerMap.insert(std::pair<int, Player*>(playerNumber++, player));
	}

	return this->playerMap.size() > 0;
}

bool InputSystem::Shutdown()
{
	for (auto pair : this->playerMap)
	{
		Player* player = pair.second;
		player->input->Shutdown();
		delete player;
	}

	this->playerMap.clear();
	return true;
}

void InputSystem::Tick(double deltaTime)
{
	for (auto pair : this->playerMap)
	{
		Player* player = pair.second;
		player->input->Update(deltaTime);
	}
}

void InputSystem::HandleInputMessage(WPARAM wParam, LPARAM lParam)
{
	for (auto pair : this->playerMap)
	{
		Player* player = pair.second;
		player->input->HandleInputMessage(wParam, lParam);
	}
}

Input* InputSystem::GetInput(int playerNumber, const std::string& user /*= ""*/)
{
	if (user.length() > 0 && this->GetCurrentUser(playerNumber) != user)
		return nullptr;

	Player* player = this->GetPlayer(playerNumber);
	if (!player)
		return nullptr;

	return player->input;
}

void InputSystem::PushUser(int playerNumber, const std::string& user)
{
	Player* player = this->GetPlayer(playerNumber);
	if (player)
		player->userStack.push_back(user);
}

std::string InputSystem::PopUser(int playerNumber)
{
	std::string user = this->GetCurrentUser(playerNumber);

	Player* player = this->GetPlayer(playerNumber);
	if (player && player->userStack.size() > 0)
		player->userStack.pop_back();
	
	return user;
}

std::string InputSystem::GetCurrentUser(int playerNumber)
{
	Player* player = this->GetPlayer(playerNumber);
	if (!player || player->userStack.size() == 0)
		return "";

	return player->userStack[player->userStack.size() - 1];
}

InputSystem::Player* InputSystem::GetPlayer(int playerNumber)
{
	PlayerMap::iterator iter = this->playerMap.find(playerNumber);
	if (iter == this->playerMap.end())
		return nullptr;

	return iter->second;
}