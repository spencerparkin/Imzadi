#include "Command.h"
#include "Thread.h"
#include "BoundingBoxTree.h"
#include <format>
#include <filesystem>
#include <fstream>

using namespace Imzadi;
using namespace Imzadi::Collision;

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
		return;

	thread->AddShape(this->shape, this->flags);
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
	if (shape)
		shape->SetDebugRenderColor(this->color);
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
		return;

	shape->SetObjectToWorldTransform(this->objectToWorld);

	BoundingBoxTree& boxTree = thread->GetBoundingBoxTree();
	boxTree.Insert(shape, 0);
}

//------------------------------- ResetProfileDataCommand -------------------------------

ResetProfileDataCommand::ResetProfileDataCommand()
{
}

/*virtual*/ ResetProfileDataCommand::~ResetProfileDataCommand()
{
}

/*virtual*/ void ResetProfileDataCommand::Execute(Thread* thread)
{
	collisionProfileData.Reset();
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

/*virtual*/ void FileCommand::Execute(Thread* thread)
{
	if (this->filePath->size() == 0)
		return;

	switch (this->action)
	{
		case Action::DUMP:
		{
			if (std::filesystem::exists(*this->filePath))
				return;

			std::ofstream stream;
			stream.open(*this->filePath, std::ios::binary);
			if (!stream.is_open())
				return;

			if (!thread->DumpShapes(stream))
				return;

			stream.close();
			break;
		}
		case Action::RESTORE:
		{
			if (!std::filesystem::exists(*this->filePath))
				return;

			std::ifstream stream;
			stream.open(*this->filePath, std::ios::binary);
			if (!stream.is_open())
				return;

			if (!thread->RestoreShapes(stream))
				return;

			stream.close();
			break;
		}
	}
}