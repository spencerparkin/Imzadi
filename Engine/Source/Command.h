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

		/**
		 * Return the name of the command.  This is also what the user types of issue the command on the command-line.
		 */
		virtual std::string GetName() = 0;

		/**
		 * Return a helpful string explaining the basic format of the command.
		 */
		virtual std::string GetSyntaxHelp() = 0;

		/**
		 * Return a short, helpful description of what the command does.
		 */
		virtual std::string GetHelpDescription() = 0;

		/**
		 * Return a detailed, multi-line help statement.
		 */
		virtual std::string GetDetailedHelp();

		/**
		 * Perform whatever work the command does.
		 * 
		 * @param[in] argument These are the arguments passed to the command.
		 * @param[out] results Populate this with a list of strings to be displayed in the console output.
		 * @return Return true if and only if the command success.  If it doesn't, command-help is presented to the user.
		 */
		virtual bool Execute(const std::vector<std::string>& arguments, std::vector<std::string>& results) = 0;

		static void Register(ConsoleCommand* command);
		static std::list<ConsoleCommand*>& GetRegistrationList();

	private:
		static std::list<ConsoleCommand*> registrationList;
	};
}