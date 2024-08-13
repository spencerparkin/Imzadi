#include "Command.h"

namespace Imzadi
{
	/**
	 * This command can be used to inspect and tweak the collision system at run-time.
	 */
	class IMZADI_API CollisionSystemCommand : public ConsoleCommand
	{
	public:
		CollisionSystemCommand();
		virtual ~CollisionSystemCommand();

		virtual std::string GetName() override;
		virtual std::string GetSyntaxHelp() override;
		virtual std::string GetHelpDescription() override;
		virtual bool Execute(const std::vector<std::string>& arguments, std::vector<std::string>& results) override;
	};
}