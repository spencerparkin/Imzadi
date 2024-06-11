#include "Command.h"
#include "Thread.h"
#include "Error.h"
#include "BoundingBoxTree.h"
#include <format>
#include <filesystem>
#include <fstream>

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
	this->flags = 0;
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

	thread->AddShape(this->shape, this->flags);
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
	if (!boxTree.Insert(shape, 0))
	{
		GetError()->AddErrorMessage(std::format("Failed to re-insert shape {} into AABB tree.", this->shapeID));
	}
}

/*static*/ ObjectToWorldCommand* ObjectToWorldCommand::Create()
{
	return new ObjectToWorldCommand();
}

//------------------------------- FileCommand -------------------------------

FileCommand::FileCommand()
{
	this->filePath = new std::string();
	this->action = Action::DUMP;
}

/*virtual*/ FileCommand::~FileCommand()
{
	delete this->filePath;
}

/*static*/ FileCommand* FileCommand::Create()
{
	return new FileCommand();
}

/*virtual*/ void FileCommand::Execute(Thread* thread)
{
	if (this->filePath->size() == 0)
	{
		GetError()->AddErrorMessage("No file path was given.");
		return;
	}

	switch (this->action)
	{
		case Action::DUMP:
		{
			if (std::filesystem::exists(*this->filePath))
			{
				GetError()->AddErrorMessage(std::format("The configured file ({}) already exists.  Can't overwrite existing file.", *this->filePath));
				return;
			}

			std::ofstream stream;
			stream.open(*this->filePath, std::ios::binary);
			if (!stream.is_open())
			{
				GetError()->AddErrorMessage(std::format("Failed to open file ({}) for writing binary.", *this->filePath));
				return;
			}

			if (!thread->DumpShapes(stream))
			{
				GetError()->AddErrorMessage(std::format("Failed to dump shapes to file: {}", *this->filePath));
				return;
			}

			stream.close();
			break;
		}
		case Action::RESTORE:
		{
			if (!std::filesystem::exists(*this->filePath))
			{
				GetError()->AddErrorMessage(std::format("The configured file ({}) does not exist.  Can't read non-existent file.", *this->filePath));
				return;
			}

			std::ifstream stream;
			stream.open(*this->filePath, std::ios::binary);
			if (!stream.is_open())
			{
				GetError()->AddErrorMessage(std::format("Failed to open file ({}) for reading binary.", *this->filePath));
				return;
			}

			if (!thread->RestoreShapes(stream))
			{
				GetError()->AddErrorMessage(std::format("Failed to restore shapes from file: {}", *this->filePath));
				return;
			}

			stream.close();
			break;
		}
	}
}