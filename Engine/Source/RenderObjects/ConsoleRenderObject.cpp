#include "ConsoleRenderObject.h"
#include "Command.h"
#include "Game.h"

using namespace Imzadi;

ConsoleRenderObject::ConsoleRenderObject()
{
	this->consoleOutput.push_back("Imzadi Engine Console");
	this->consoleOutput.push_back("");
	this->consoleOutput.push_back("Enter \"help\" for a list of commands.");
	this->consoleOutput.push_back("");

	this->scrollPosition = 0;
	this->maxLinesToShow = 16;

	this->shiftMap.insert(std::pair<char, char>('`', '~'));
	this->shiftMap.insert(std::pair<char, char>('1', '!'));
	this->shiftMap.insert(std::pair<char, char>('2', '@'));
	this->shiftMap.insert(std::pair<char, char>('3', '#'));
	this->shiftMap.insert(std::pair<char, char>('4', '$'));
	this->shiftMap.insert(std::pair<char, char>('5', '%'));
	this->shiftMap.insert(std::pair<char, char>('6', '^'));
	this->shiftMap.insert(std::pair<char, char>('7', '&'));
	this->shiftMap.insert(std::pair<char, char>('8', '*'));
	this->shiftMap.insert(std::pair<char, char>('9', '('));
	this->shiftMap.insert(std::pair<char, char>('0', ')'));
	this->shiftMap.insert(std::pair<char, char>('-', '_'));
	this->shiftMap.insert(std::pair<char, char>('=', '+'));
	this->shiftMap.insert(std::pair<char, char>('[', '{'));
	this->shiftMap.insert(std::pair<char, char>(']', '}'));
	this->shiftMap.insert(std::pair<char, char>(';', ':'));
	this->shiftMap.insert(std::pair<char, char>('\'', '"'));
	this->shiftMap.insert(std::pair<char, char>(',', '<'));
	this->shiftMap.insert(std::pair<char, char>('.', '>'));
	this->shiftMap.insert(std::pair<char, char>('/', '?'));
}

/*virtual*/ ConsoleRenderObject::~ConsoleRenderObject()
{
}

/*virtual*/ void ConsoleRenderObject::OnPostAdded()
{
	this->commandMap.clear();

	for (ConsoleCommand* command : ConsoleCommand::GetRegistrationList())
	{
		std::string name = command->GetName();
		IMZADI_ASSERT(this->commandMap.find(name) == this->commandMap.end());
		this->commandMap.insert(std::pair<std::string, ConsoleCommand*>(name, command));
	}
}

void ConsoleRenderObject::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case VK_RETURN:
		{
			this->ExecuteCommand();
			break;
		}
		case VK_BACK:
		{
			if (this->consoleInput.size() > 0)
				this->consoleInput.pop_back();
			break;
		}
		case VK_PRIOR:
		{
			if (this->scrollPosition >= this->maxLinesToShow / 2)
				this->scrollPosition -= this->maxLinesToShow / 2;
			else
				this->scrollPosition = 0;
			break;
		}
		case VK_NEXT:
		{
			if ((signed)this->consoleOutput.size() > this->maxLinesToShow / 2)
			{
				if (this->scrollPosition < (signed)this->consoleOutput.size() - this->maxLinesToShow / 2)
					this->scrollPosition += this->maxLinesToShow / 2;
				else
					this->scrollPosition = (signed)this->consoleInput.size() - this->maxLinesToShow / 2;
			}
			break;
		}
		default:
		{
			unsigned char ch = (unsigned char)wParam;

			if ('A' <= ch && ch <= 'Z')
			{
				if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
					this->consoleInput += ch;
				else
					this->consoleInput += ch + 32;
			}
			else if ((32 <= ch && ch <= 64) || (91 <= ch && ch <= 96) || (123 <= ch && ch <= 126))
			{
				if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
				{
					std::unordered_map<char, char>::iterator iter = this->shiftMap.find(ch);
					if (iter != this->shiftMap.end())
						ch = iter->second;
				}

				this->consoleInput += ch;
			}

			break;
		}
	}
}

void ConsoleRenderObject::ExecuteCommand()
{
	std::vector<std::string> arguments;
	this->SplitString(this->consoleInput, arguments);

	if (arguments.size() == 0)
		return;

	std::string commandName = arguments[0];
	arguments.erase(arguments.begin());

	if (commandName == "help")
	{
		if (arguments.size() == 1)
		{
			commandName = arguments[0];
			std::map<std::string, ConsoleCommand*>::iterator iter = this->commandMap.find(commandName);
			if(iter == this->commandMap.end())
				this->consoleOutput.push_back(std::format("No command with name \"{}\" found.", commandName.c_str()));
			else
			{
				ConsoleCommand* command = iter->second;
				this->consoleOutput.push_back(command->GetSyntaxHelp());
				std::string detailedHelp = command->GetDetailedHelp();
				std::vector<std::string> lineArray;
				this->SplitString(detailedHelp, lineArray, "\n");
				for (const std::string& line : lineArray)
					this->consoleOutput.push_back(line);
			}
		}
		else
		{
			for (auto pair : this->commandMap)
			{
				ConsoleCommand* command = pair.second;
				const std::string& commandName = pair.first;
				this->consoleOutput.push_back(commandName + " - " + command->GetHelpDescription());
			}
		}
	}
	else
	{
		std::map<std::string, ConsoleCommand*>::iterator iter = this->commandMap.find(commandName);
		if (iter == this->commandMap.end())
			this->consoleOutput.push_back(std::format("No command with name \"{}\" found.", commandName.c_str()));
		else
		{
			ConsoleCommand* command = iter->second;
			std::vector<std::string> results;
			if (!command->Execute(arguments, results))
			{
				this->consoleOutput.push_back(command->GetSyntaxHelp());
				this->consoleOutput.push_back(command->GetHelpDescription());
			}
			else
			{
				std::vector<std::string> outputLineArray;
				for (const std::string& result : results)
					this->SplitString(result, outputLineArray, "\n");

				for (const std::string& outputLine : outputLineArray)
					this->consoleOutput.push_back(outputLine);
			}
		}
	}

	this->scrollPosition = (signed)this->consoleOutput.size() - this->maxLinesToShow;
	if (this->scrollPosition < 0)
		this->scrollPosition = 0;

	this->consoleInput = "";
}

/*virtual*/ void ConsoleRenderObject::Prepare()
{
	std::string text = "";

	for (int i = this->scrollPosition; 0 <= i && i < (signed)this->consoleOutput.size(); i++)
	{
		if (i - this->scrollPosition >= this->maxLinesToShow)
			break;

		text += this->consoleOutput[i] + "\n";
	}

	text += "Input: " + this->consoleInput;

	this->SetText(text);

	uint32_t flags = 0;
	flags |= Flag::ALWAYS_ON_TOP;
	flags |= Flag::LEFT_JUSTIFY;
	flags |= Flag::STICK_WITH_CAMERA_PROJ;
	flags |= Flag::MULTI_LINE;
	flags |= Flag::USE_NEWLINE_CHARS;
	flags |= Flag::DRAW_BACKGROUND;
	this->SetFlags(flags);

	this->SetBackgroundColor(Vector3(0.0, 0.0, 0.0));
	this->SetForegroundColor(Vector3(1.0, 1.0, 1.0));
	this->SetBackgroundAlpha(0.85);

	const D3D11_VIEWPORT* viewport = Game::Get()->GetViewportInfo();
	double aspectRatio = double(viewport->Width) / double(viewport->Height);

	Transform scale;
	scale.SetIdentity();
	scale.matrix.SetNonUniformScale(Vector3(1.0, aspectRatio, 1.0));

	Transform transform;
	transform.SetIdentity();
	transform.translation.SetComponents(-0.9, -0.9, -0.5);

	this->SetTransform(transform * scale);

	TextRenderObject::Prepare();
}