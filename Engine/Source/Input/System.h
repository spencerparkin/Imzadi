#pragma once

#include "Defines.h"
#include "Input.h"
#include <string>
#include <vector>
#include <map>

namespace Imzadi
{
	class Input;

	/**
	 * This class manages input sources for the game.
	 */
	class IMZADI_API InputSystem
	{
	public:
		InputSystem();
		virtual ~InputSystem();

		bool Setup(HWND windowHandle);

		bool Shutdown();

		void Tick(double deltaTime);

		void HandleInputMessage(WPARAM wParam, LPARAM lParam);

		Input* GetInput(int playerNumber, const std::string& user = "");

		void PushUser(int playerNumber, const std::string& user);
		std::string PopUser(int playerNumber);

		std::string GetCurrentUser(int playerNumber);

		// TODO: Provide mapping system for the buttons?  E.g., map "jump" string to Y_BUTTON, etc.
		//       Be able to save/restore the user's key-bindings/mapping settings.

		// TODO: What about the ability to detect when a controller is plugged in or removed?
		//       A more robust game engine would be able to handle all that sort of stuff.
		//       Note that a message could be sent when a controller is added or removed.

	private:

		struct Player
		{
			Input* input;
			std::vector<std::string> userStack;
		};

		template<typename T>
		Player* MakePlayer(int playerNumber, HWND windowHandle)
		{
			auto player = new Player();
			player->input = new T(playerNumber);
			if (!player->input->Setup(windowHandle))
			{
				delete player->input;
				delete player;
				player = nullptr;
			}

			return player;
		}

		Player* GetPlayer(int playerNumber);

		typedef std::map<int, Player*> PlayerMap;
		PlayerMap playerMap;
	};
}