#include "System.h"
#include "Result.h"
#include "Thread.h"
#include "Query.h"
#include "Command.h"

using namespace Imzadi;
using namespace Imzadi::Collision;

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
		return false;

	this->thread = new Thread(collsionWorldExtents);

	if (!this->thread->Startup())
	{
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

ShapeID System::AddShape(Shape* shape, uint32_t flags)
{
	if (!this->thread)
		return false;

	if (!shape)
		return false;

	ShapeID shapeID = shape->GetShapeID();

	auto command = AddShapeCommand::Create();
	command->SetShape(shape);
	command->SetFlags(flags);
	this->thread->SendTask(command);

	return shapeID;
}

void System::RemoveShape(ShapeID shapeID)
{
	auto command = this->Create<RemoveShapeCommand>();
	command->SetShapeID(shapeID);
	this->IssueCommand(command);
}

void System::Clear()
{
	this->IssueCommand(this->Create<RemoveAllShapesCommand>());
}

bool System::IssueCommand(Command* command)
{
	if (!this->thread)
		return false;

	this->thread->SendTask(command);
	return true;
}

bool System::MakeQuery(Query* query, TaskID& taskID)
{
	if (!this->thread)
		return false;

	taskID = this->thread->SendTask(query);
	return true;
}

Result* System::ObtainQueryResult(TaskID taskID)
{
	if (!this->thread)
		return nullptr;

	return this->thread->ReceiveResult(taskID);
}

bool System::FlushAllTasks()
{
	if (!this->thread)
		return false;

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
	return true;
}

bool System::RestoreFromFile(const std::string& fileName)
{
	auto command = FileCommand::Create();
	command->SetFilePath(fileName);
	command->SetAction(FileCommand::Action::RESTORE);
	this->IssueCommand(command);
	this->FlushAllTasks();
	return true;
}