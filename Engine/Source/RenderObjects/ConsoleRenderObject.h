#pragma once

#include "TextRenderObject.h"
#include <map>

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

	private:
		std::map<std::string, ConsoleCommand*> commandMap;
	};
}