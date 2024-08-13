#pragma once

#include "Defines.h"
#include <vector>
#include <string>

namespace Imzadi
{
	/**
	 * This is the base class for all command implimentations.  These are commands
	 * that can be executed from the game's console.
	 */
	class IMZADI_API ConsoleCommand
	{
	public:
		ConsoleCommand();
		virtual ~ConsoleCommand();

		virtual std::string GetName() = 0;
		virtual std::string GetSyntaxHelp() = 0;
		virtual std::string GetHelpDescription() = 0;
		virtual void Execute(const std::vector<std::string>& arguments) = 0;

		static void Register(ConsoleCommand* command);
		static std::list<ConsoleCommand*>& GetRegistrationList();

	private:
		static std::list<ConsoleCommand*> registrationList;
	};
}