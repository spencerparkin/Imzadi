#include "System.h"
#include "Result.h"
#include "Error.h"
#include "Thread.h"
#include "Query.h"
#include "Command.h"

using namespace Collision;

System::System()
{
	this->thread = nullptr;
}

/*virtual*/ System::~System()
{
	delete this->thread;
}

bool System::Initialize(const AxisAlignedBoundingBox& collsionWorldExtents)
{
	if (this->thread)
	{
		GetError()->AddErrorMessage("Collision thread already created!");
		return false;
	}

	this->thread = new Thread(collsionWorldExtents);

	if (!this->thread->Startup())
	{
		GetError()->AddErrorMessage("Collision thread failed to start.");

		delete this->thread;
		this->thread = nullptr;

		return false;
	}

	return true;
}

bool System::Shutdown()
{
	if (this->thread)
	{
		this->thread->Shutdown();
		delete this->thread;
		this->thread = nullptr;
	}

	return true;
}

ShapeID System::AddShape(Shape* shape)
{
	if (!this->thread)
	{
		GetError()->AddErrorMessage("Can't add a shape if the collision thread isn't running.");
		return false;
	}

	if (!shape)
	{
		GetError()->AddErrorMessage("Shape given to add-shape function was null.");
		return false;
	}

	ShapeID shapeID = shape->GetShapeID();

	auto command = AddShapeCommand::Create();
	command->SetShape(shape);
	this->thread->SendTask(command);

	return shapeID;
}

void System::Clear()
{
}

bool System::IssueCommand(Command* command)
{
	if (!this->thread)
	{
		GetError()->AddErrorMessage("Can't issue a command if the collision thread isn't running.");
		return false;
	}

	this->thread->SendTask(command);
	return true;
}

bool System::MakeQuery(Query* query, TaskID& taskID)
{
	if (!this->thread)
	{
		GetError()->AddErrorMessage("Can't submit query if the collision thread isn't running.");
		return false;
	}

	taskID = this->thread->SendTask(query);
	return true;
}

Result* System::ObtainQueryResult(TaskID taskID)
{
	if (!this->thread)
	{
		GetError()->AddErrorMessage("Can't object any result if the collision thread isn't running.");
		return nullptr;
	}

	return this->thread->ReceiveResult(taskID);
}

bool System::FlushAllTasks()
{
	if (!this->thread)
	{
		GetError()->AddErrorMessage("Collision thread not running!");
		return false;
	}

	this->thread->WaitForAllTasksToComplete();
	return true;
}

bool System::DumpToFile(const std::string& fileName)
{
	auto command = FileCommand::Create();
	command->SetFilePath(fileName);
	command->SetAction(FileCommand::Action::DUMP);
	this->IssueCommand(command);
	this->FlushAllTasks();
	return GetError()->GetCount() == 0;
}

bool System::RestoreFromFile(const std::string& fileName)
{
	auto command = FileCommand::Create();
	command->SetFilePath(fileName);
	command->SetAction(FileCommand::Action::RESTORE);
	this->IssueCommand(command);
	this->FlushAllTasks();
	return GetError()->GetCount() == 0;
}