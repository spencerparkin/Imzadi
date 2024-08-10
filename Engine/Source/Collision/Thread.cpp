#include "Thread.h"
#include "Result.h"
#include "Command.h"
#include "Query.h"
#include "Shape.h"
#include <format>
#include <ostream>
#include <istream>

using namespace Imzadi;

namespace Imzadi
{
	ProfileData collisionProfileData;
}

Thread::Thread(const AxisAlignedBoundingBox& collisionWorldExtents) : boxTree(collisionWorldExtents), taskQueueSemaphore(0)
{
	this->thread = nullptr;
	this->signaledToExit = false;
}

/*virtual*/ Thread::~Thread()
{
}

bool Thread::Startup()
{
	if (this->thread)
		return false;

	this->signaledToExit = false;
	this->thread = new std::thread(&Thread::EntryFunc, this);

	return true;
}

bool Thread::Shutdown()
{
	if (this->thread)
	{
		this->SendTask(ExitThreadCommand::Create());
		this->thread->join();
		delete this->thread;
		this->thread = nullptr;
	}

	return true;
}

/*static*/ void Thread::EntryFunc(Thread* thread)
{
	thread->Run();
}

void Thread::Run()
{
	while (!this->signaledToExit)
	{
		// Don't eat up any CPU resources if there are no tasks queued.
		// The semaphore count mirrors the size of the task queue.
		this->taskQueueSemaphore.acquire();

		// Grab the next task, but don't pull it off the queue just yet.
		Task* task = nullptr;
		if (this->taskQueue.size() > 0)
		{
			std::lock_guard<std::mutex> guard(this->taskQueueMutex);
			if (this->taskQueue.size() > 0)
			{
				std::list<Task*>::iterator iter = this->taskQueue.begin();
				task = *iter;
			}
		}

		if (task)
		{
			IMZADI_COLLISION_PROFILE("Task Execution");

			// Process the task.
			task->Execute(this);
			Task::Free(task);
			task = nullptr;
		
			// Once processed, we can remove it from the queue.  This way, our wait
			// operation returns when all tasks are truely completed.
			{
				std::lock_guard<std::mutex> guard(this->taskQueueMutex);
				std::list<Task*>::iterator iter = this->taskQueue.begin();
				this->taskQueue.erase(iter);
				if (this->taskQueue.size() == 0)
					this->allTasksDoneCondVar.notify_one();
			}
		}
	}

	this->ClearTasks();
	this->ClearResults();
	this->ClearShapes();
}

void Thread::ClearTasks()
{
	std::lock_guard<std::mutex> guard(this->taskQueueMutex);
	while (this->taskQueue.size() > 0)
	{
		std::list<Task*>::iterator iter = this->taskQueue.begin();
		Task* task = *iter;
		Task::Free(task);
		this->taskQueue.erase(iter);
	}
}

void Thread::ClearResults()
{
	std::lock_guard<std::mutex> guard(this->resultMapMutex);
	while (this->resultMap.size() > 0)
	{
		std::unordered_map<TaskID, Result*>::iterator iter = this->resultMap.begin();
		Result* result = iter->second;
		Result::Free(result);
		this->resultMap.erase(iter);
	}
}

void Thread::ClearShapes()
{
	this->boxTree.Clear();
}

void Thread::AddShape(Shape* shape, uint32_t flags)
{
	ShapeID shapeID = shape->GetShapeID();

	if (!this->boxTree.Insert(shape, flags))
		Shape::Free(shape);
}

void Thread::RemoveShape(ShapeID shapeID)
{
	this->boxTree.Remove(shapeID);
}

Shape* Thread::FindShape(ShapeID shapeID)
{
	Shape* shape = this->boxTree.FindShape(shapeID);
	IMZADI_ASSERT(!shape || shape->GetShapeID() == shapeID);
	return shape;
}

TaskID Thread::SendTask(Task* task)
{
	TaskID taskId = task->GetTaskID();

	// Add the task to the queue.  Make the mutex scope-lock as tight as possible.
	{
		std::lock_guard<std::mutex> guard(this->taskQueueMutex);
		this->taskQueue.push_back(task);
	}

	// Signal the collision thread that a task is available.
	this->taskQueueSemaphore.release();

	return taskId;
}

Result* Thread::ReceiveResult(TaskID taskID)
{
	Result* result = nullptr;

	// Look-up the result, if it's ready.  Make the mutex scope-lock as tight as possible.
	{
		std::lock_guard<std::mutex> guard(this->resultMapMutex);
		std::unordered_map<TaskID, Result*>::iterator iter = this->resultMap.find(taskID);
		if (iter != this->resultMap.end())
		{
			result = iter->second;
			this->resultMap.erase(iter);
		}
	}

	return result;
}

void Thread::StoreResult(Result* result, TaskID taskID)
{
	std::lock_guard<std::mutex> guard(this->resultMapMutex);
	std::unordered_map<TaskID, Result*>::iterator iter = this->resultMap.find(taskID);
	if (iter == this->resultMap.end())
		this->resultMap.insert(std::pair<TaskID, Result*>(taskID, result));
	else
	{
		Result::Free(iter->second);
		iter->second = result;
	}
}

void Thread::DebugVisualize(DebugRenderResult* renderResult, uint32_t drawFlags)
{
	if ((drawFlags & IMZADI_DRAW_FLAG_SHAPES) != 0)
	{
		this->boxTree.ForAllShapes([renderResult, drawFlags](const Shape* shape) -> bool
		{
			shape->DebugRender(renderResult);
			if ((drawFlags & IMZADI_DRAW_FLAG_SHAPE_BOXES) != 0)
				renderResult->AddLinesForBox(shape->GetBoundingBox(), Vector3(1.0, 1.0, 1.0) - shape->GetDebugRenderColor());
			return true;
		});
	}

	if ((drawFlags & IMZADI_DRAW_FLAG_AABB_TREE) != 0)
		this->boxTree.DebugRender(renderResult);
}

void Thread::WaitForAllTasksToComplete()
{
	std::unique_lock<std::mutex> lock(this->taskQueueMutex);
	this->allTasksDoneCondVar.wait(lock, [=]() { return this->taskQueue.size() == 0; });
}

bool Thread::DumpShapes(std::ostream& stream, const std::vector<const Shape*>* shapeArray /*= nullptr*/) const
{
	std::vector<const Shape*> allShapesArray;
	const std::vector<const Shape*>* shapesToDumpArray = shapeArray;
	if (!shapesToDumpArray)
	{
		this->boxTree.ForAllShapes([&allShapesArray](const Shape* shape) -> bool
		{
			allShapesArray.push_back(shape);
			return true;
		});
		shapesToDumpArray = &allShapesArray;
	}

	uint32_t numShapes = (uint32_t)shapesToDumpArray->size();
	stream.write((char*)&numShapes, sizeof(uint32_t));

	for(const Shape* shape : *shapesToDumpArray)
	{
		uint32_t typeID = shape->GetShapeTypeID();
		stream.write((char*)&typeID, sizeof(typeID));
		if (!shape->Dump(stream))
			return false;
	}

	return true;
}

bool Thread::RestoreShapes(std::istream& stream)
{
	this->ClearShapes();

	uint32_t numShapes = 0;
	stream.read((char*)&numShapes, sizeof(numShapes));

	for (uint32_t i = 0; i < numShapes; i++)
	{
		uint32_t typeID = 0;
		stream.read((char*)&typeID, sizeof(typeID));
		Shape* shape = Shape::Create((Shape::TypeID)typeID);
		if (!shape)
			return false;
		
		if (!shape->Restore(stream))
			return false;

		this->AddShape(shape, 0);
	}

	return true;
}