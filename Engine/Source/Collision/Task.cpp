#include "Task.h"

using namespace Imzadi;

TaskID Task::nextTaskID = 0;

Task::Task()
{
	this->taskID = nextTaskID++;
	this->priority = 0;
}

/*virtual*/ Task::~Task()
{
}

/*static*/ void Task::Free(Task* task)
{
	delete task;
}