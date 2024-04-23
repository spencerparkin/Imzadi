#include "Result.h"

using namespace Collision;

Result::Result()
{
}

/*virtual*/ Result::~Result()
{
}

/*static*/ void Result::Free(Result* result)
{
	delete result;
}