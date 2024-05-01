#include "Command.h"
#include "Thread.h"
#include "Error.h"
#include "BoundingBoxTree.h"
#include <format>

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

//------------------------------- ShapeCommand -------------------------------

ShapeCommand::ShapeCommand()
{
	this->shapeID = 0;
}

/*virtual*/ ShapeCommand::~ShapeCommand()
{
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

//------------------------------- RemoveAllShapesCommand -------------------------------

SetDebugRenderColorCommand::SetDebugRenderColorCommand()
{
}

/*virtual*/ SetDebugRenderColorCommand::~SetDebugRenderColorCommand()
{
}

/*virtual*/ void SetDebugRenderColorCommand::Execute(Thread* thread)
{
	Shape* shape = thread->FindShape(this->shapeID);
	if (!shape)
		GetError()->AddErrorMessage(std::format("Failed to find shape with ID {}.", this->shapeID));
	else
		shape->SetDebugRenderColor(this->color);
}

/*static*/ SetDebugRenderColorCommand* SetDebugRenderColorCommand::Create()
{
	return new SetDebugRenderColorCommand();
}

//------------------------------- ObjectToWorldCommand -------------------------------

ObjectToWorldCommand::ObjectToWorldCommand()
{
	this->objectToWorld.SetIdentity();
}

/*virtual*/ ObjectToWorldCommand::~ObjectToWorldCommand()
{
}

/*virtual*/ void ObjectToWorldCommand::Execute(Thread* thread)
{
	Shape* shape = thread->FindShape(this->shapeID);
	if (!shape)
	{
		GetError()->AddErrorMessage(std::format("Failed to find shape with ID {}.", this->shapeID));
		return;
	}

	shape->SetObjectToWorldTransform(this->objectToWorld);

	BoundingBoxTree& boxTree = thread->GetBoundingBoxTree();
	if (!boxTree.Insert(shape))
	{
		GetError()->AddErrorMessage(std::format("Failed to re-insert shape {} into AABB tree.", this->shapeID));
	}
}