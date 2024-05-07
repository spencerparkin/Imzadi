#include "Thread.h"
#include "Error.h"
#include "Result.h"
#include "Command.h"
#include "Query.h"
#include "Shape.h"
#include <format>
#include <ostream>
#include <istream>

using namespace Collision;

Thread::Thread(const AxisAlignedBoundingBox& collisionWorldExtents) : boxTree(collisionWorldExtents)
{
	this->thread = nullptr;
	this->signaledToExit = false;
	this->taskQueue = new std::list<Task*>();
	this->taskQueueMutex = new std::mutex();
	this->taskQueueSemaphore = new std::counting_semaphore<4096>(0);
	this->resultMap = new std::unordered_map<TaskID, Result*>();
	this->resultMapMutex = new std::mutex();
	this->shapeMap = new std::unordered_map<ShapeID, Shape*>();
	this->allTasksDoneCondVar = new std::condition_variable();
}

/*virtual*/ Thread::~Thread()
{
	delete this->taskQueue;
	delete this->taskQueueMutex;
	delete this->taskQueueSemaphore;
	delete this->resultMap;
	delete this->resultMapMutex;
	delete this->shapeMap;
	delete this->allTasksDoneCondVar;
}

bool Thread::Startup()
{
	if (this->thread)
	{
		GetError()->AddErrorMessage("Thread already created!");
		return false;
	}

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
		this->taskQueueSemaphore->acquire();

		// Grab the next task, but don't pull it off the queue just yet.
		Task* task = nullptr;
		if (this->taskQueue->size() > 0)
		{
			std::lock_guard<std::mutex> guard(*this->taskQueueMutex);
			if (this->taskQueue->size() > 0)
			{
				std::list<Task*>::iterator iter = this->taskQueue->begin();
				task = *iter;
			}
		}

		if (task)
		{
			// Process the task.
			task->Execute(this);
			Task::Free(task);
			task = nullptr;
		
			// Once processed, we can remove it from the queue.  This way, our wait
			// operation returns when all tasks are truely completed.
			{
				std::lock_guard<std::mutex> guard(*this->taskQueueMutex);
				std::list<Task*>::iterator iter = this->taskQueue->begin();
				this->taskQueue->erase(iter);
				if (this->taskQueue->size() == 0)
					this->allTasksDoneCondVar->notify_one();
			}
		}
	}

	this->ClearTasks();
	this->ClearResults();
	this->ClearShapes();
}

void Thread::ClearTasks()
{
	std::lock_guard<std::mutex> guard(*this->taskQueueMutex);
	while (this->taskQueue->size() > 0)
	{
		std::list<Task*>::iterator iter = this->taskQueue->begin();
		Task* task = *iter;
		Task::Free(task);
		this->taskQueue->erase(iter);
	}
}

void Thread::ClearResults()
{
	std::lock_guard<std::mutex> guard(*this->resultMapMutex);
	while (this->resultMap->size() > 0)
	{
		std::unordered_map<TaskID, Result*>::iterator iter = this->resultMap->begin();
		Result* result = iter->second;
		Result::Free(result);
		this->resultMap->erase(iter);
	}
}

void Thread::ClearShapes()
{
	this->boxTree.Clear();

	while (this->shapeMap->size() > 0)
	{
		std::unordered_map<ShapeID, Shape*>::iterator iter = this->shapeMap->begin();
		Shape* shape = iter->second;
		Shape::Free(shape);
		this->shapeMap->erase(iter);
	}
}

void Thread::AddShape(Shape* shape)
{
	std::unordered_map<ShapeID, Shape*>::iterator iter = this->shapeMap->find(shape->GetShapeID());
	if (iter == this->shapeMap->end())
	{
		this->shapeMap->insert(std::pair<ShapeID, Shape*>(shape->GetShapeID(), shape));
		this->boxTree.Insert(shape);
	}
	else
	{
		GetError()->AddErrorMessage(std::format("Cannot add shape.  A shape with ID {} already exists in the system.", shape->GetShapeID()));
		Shape::Free(shape);
	}
}

void Thread::RemoveShape(ShapeID shapeID)
{
	std::unordered_map<ShapeID, Shape*>::iterator iter = this->shapeMap->find(shapeID);
	if (iter != this->shapeMap->end())
	{
		Shape* shape = iter->second;
		this->boxTree.Remove(shape);
		Shape::Free(shape);
		this->shapeMap->erase(iter);
	}
	else
	{
		GetError()->AddErrorMessage(std::format("Cannot remove shape.  No shape with ID {} was found in the system.", shapeID));
	}
}

Shape* Thread::FindShape(ShapeID shapeID)
{
	Shape* shape = nullptr;
	std::unordered_map<ShapeID, Shape*>::iterator iter = this->shapeMap->find(shapeID);
	if (iter != this->shapeMap->end())
	{
		shape = iter->second;
		COLL_SYS_ASSERT(shape->GetShapeID() == shapeID);
	}

	return shape;
}

TaskID Thread::SendTask(Task* task)
{
	TaskID taskId = task->GetTaskID();

	// Add the task to the queue.  Make the mutex scope-lock as tight as possible.
	{
		std::lock_guard<std::mutex> guard(*this->taskQueueMutex);
		this->taskQueue->push_back(task);
	}

	// Signal the collision thread that a task is available.
	this->taskQueueSemaphore->release();

	return taskId;
}

Result* Thread::ReceiveResult(TaskID taskID)
{
	Result* result = nullptr;

	// Look-up the result, if it's ready.  Make the mutex scope-lock as tight as possible.
	{
		std::lock_guard<std::mutex> guard(*this->resultMapMutex);
		std::unordered_map<TaskID, Result*>::iterator iter = this->resultMap->find(taskID);
		if (iter != this->resultMap->end())
		{
			result = iter->second;
			this->resultMap->erase(iter);
		}
	}

	return result;
}

void Thread::StoreResult(Result* result, TaskID taskID)
{
	std::lock_guard<std::mutex> guard(*this->resultMapMutex);
	std::unordered_map<TaskID, Result*>::iterator iter = this->resultMap->find(taskID);
	if (iter == this->resultMap->end())
		this->resultMap->insert(std::pair<TaskID, Result*>(taskID, result));
	else
	{
		Result::Free(iter->second);
		iter->second = result;
	}
}

void Thread::DebugVisualize(DebugRenderResult* renderResult, uint32_t drawFlags)
{
	if ((drawFlags & COLL_SYS_DRAW_FLAG_SHAPES) != 0)
	{
		for (auto pair : *this->shapeMap)
		{
			const Shape* shape = pair.second;
			shape->DebugRender(renderResult);

			if ((drawFlags & COLL_SYS_DRAW_FLAG_SHAPE_BOXES) != 0)
				renderResult->AddLinesForBox(shape->GetBoundingBox(), Vector3(1.0, 1.0, 1.0) - shape->GetDebugRenderColor());
		}
	}

	if ((drawFlags & COLL_SYS_DRAW_FLAG_AABB_TREE) != 0)
		this->boxTree.DebugRender(renderResult);
}

void Thread::WaitForAllTasksToComplete()
{
	std::unique_lock<std::mutex> lock(*this->taskQueueMutex);
	this->allTasksDoneCondVar->wait(lock, [=]() { return this->taskQueue->size() == 0; });
}

bool Thread::DumpShapes(std::ostream& stream) const
{
	uint32_t numShapes = (uint32_t)this->shapeMap->size();
	stream.write((char*)&numShapes, sizeof(uint32_t));

	for (auto pair : *this->shapeMap)
	{
		const Shape* shape = pair.second;
		stream << (uint32_t)shape->GetShapeTypeID();
		if (!shape->Dump(stream))
		{
			GetError()->AddErrorMessage(std::format("Failed to dump shape with ID {}.", shape->GetShapeID()));
			return false;
		}
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
		stream >> typeID;
		Shape* shape = Shape::Create((Shape::TypeID)typeID);
		if (!shape)
		{
			GetError()->AddErrorMessage(std::format("Failed to create shape with type ID {}.", typeID));
			return false;
		}
		
		if (!shape->Restore(stream))
		{
			GetError()->AddErrorMessage(std::format("Failed to restore shape with type ID {}.", typeID));
			return false;
		}

		this->AddShape(shape);
	}

	return true;
}