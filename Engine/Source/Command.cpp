#include "Command.h"

using namespace Imzadi;

namespace Imzadi
{
	std::list<ConsoleCommand*> ConsoleCommand::registrationList;
}

ConsoleCommand::ConsoleCommand()
{
	this->Register(this);
}

/*virtual*/ ConsoleCommand::~ConsoleCommand()
{
}

/*static*/ void ConsoleCommand::Register(ConsoleCommand* command)
{
	registrationList.push_back(command);
}

/*static*/ std::list<ConsoleCommand*>& ConsoleCommand::GetRegistrationList()
{
	return registrationList;
}

/*virtual*/ std::string ConsoleCommand::GetDetailedHelp()
{
	return "No detailed help available.";
}