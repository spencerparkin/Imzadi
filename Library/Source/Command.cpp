#include "Command.h"
#include "Thread.h"
#include "Error.h"

using namespace Collision;

//------------------------------- Command -------------------------------

Command::Command()
{
}

/*virtual*/ Command::~Command()
{
}

//------------------------------- ExitThreadCommand -------------------------------

ExitThreadCommand::ExitThreadCommand()
{

}

/*virtual*/ ExitThreadCommand::~ExitThreadCommand()
{
}

/*virtual*/ void ExitThreadCommand::Execute(Thread* thread)
{
	thread->signaledToExit = true;
}

/*static*/ ExitThreadCommand* ExitThreadCommand::Create()
{
	return new ExitThreadCommand();
}

//------------------------------- AddShapeCommand -------------------------------

AddShapeCommand::AddShapeCommand()
{
	this->shape = nullptr;
}

/*virtual*/ AddShapeCommand::~AddShapeCommand()
{
}

/*virtual*/ void AddShapeCommand::Execute(Thread* thread)
{
	if (!this->shape)
	{
		GetError()->AddErrorMessage("No shape was set on an add-shape command.  There is no shape to add.");
		return;
	}

	thread->AddShape(this->shape);
}

/*static*/ AddShapeCommand* AddShapeCommand::Create()
{
	return new AddShapeCommand();
}

//------------------------------- RemoveShapeCommand -------------------------------

RemoveShapeCommand::RemoveShapeCommand()
{
	this->shapeID = 0;
}

/*virtual*/ RemoveShapeCommand::~RemoveShapeCommand()
{
}

/*virtual*/ void RemoveShapeCommand::Execute(Thread* thread)
{
	thread->RemoveShape(this->shapeID);
}

/*static*/ RemoveShapeCommand* RemoveShapeCommand::Create()
{
	return new RemoveShapeCommand();
}

//------------------------------- RemoveAllShapesCommand -------------------------------

RemoveAllShapesCommand::RemoveAllShapesCommand()
{
}

/*virtual*/ RemoveAllShapesCommand::~RemoveAllShapesCommand()
{
}

/*virtual*/ void RemoveAllShapesCommand::Execute(Thread* thread)
{
	thread->ClearShapes();
}

/*static*/ RemoveAllShapesCommand* RemoveAllShapesCommand::Create()
{
	return new RemoveAllShapesCommand();
}