#include "ConsoleRenderObject.h"
#include "Command.h"
#include "Game.h"

using namespace Imzadi;

ConsoleRenderObject::ConsoleRenderObject()
{
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

/*virtual*/ void ConsoleRenderObject::Prepare()
{
	std::string debugText =
		"This is a line of text.\n"
		"This is another line of text.\n"
		"Here's some more text.\n"
		"Console: ";

	this->SetText(debugText);

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
	this->SetBackgroundAlpha(0.5);

	const D3D11_VIEWPORT* viewport = Game::Get()->GetViewportInfo();
	double aspectRatio = double(viewport->Width) / double(viewport->Height);

	Transform scale;
	scale.SetIdentity();
	scale.matrix.SetNonUniformScale(Vector3(1.0, aspectRatio, 1.0));

	Transform transform;
	transform.SetIdentity();
	transform.translation.SetComponents(-0.6, -0.9, -0.5);

	this->SetTransform(transform * scale);

	TextRenderObject::Prepare();
}