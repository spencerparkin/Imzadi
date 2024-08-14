#pragma once

#include "TextRenderObject.h"
#include <map>
#include <vector>
#include <unordered_map>

namespace Imzadi
{
	class ConsoleCommand;

	class IMZADI_API ConsoleRenderObject : public TextRenderObject
	{
	public:
		ConsoleRenderObject();
		virtual ~ConsoleRenderObject();

		virtual void Prepare() override;
		virtual void OnPostAdded() override;

		void OnKeyDown(WPARAM wParam, LPARAM lParam);

	private:
		void ExecuteCommand();

		std::map<std::string, ConsoleCommand*> commandMap;
		std::string consoleInput;
		std::vector<std::string> consoleOutput;
		int scrollPosition;
		int maxLinesToShow;
		std::unordered_map<char, char> shiftMap;
	};
}