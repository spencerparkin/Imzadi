#include "Query.h"
#include "Thread.h"

using namespace Collision;

Query::Query()
{
}

/*virtual*/ Query::~Query()
{
}

/*virtual*/ void Query::Execute(Thread* thread)
{
	Result* result = this->ExecuteQuery(thread);
	if (result)
		thread->StoreResult(result, this->GetTaskID());
}