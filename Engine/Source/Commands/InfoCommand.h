#include "Command.h"

namespace Imzadi
{
	/**
	 * This command can be used to inspect the state of entities or other things.
	 */
	class IMZADI_API InfoCommand : public ConsoleCommand
	{
	public:
		InfoCommand();
		virtual ~InfoCommand();

		virtual std::string GetName() override;
		virtual std::string GetSyntaxHelp() override;
		virtual std::string GetHelpDescription() override;
		virtual bool Execute(const std::vector<std::string>& arguments, std::vector<std::string>& results) override;
	};
}