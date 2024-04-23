#include "Result.h"

using namespace Collision;

//-------------------------------- Result --------------------------------

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

//-------------------------------- DebugRenderResult --------------------------------

DebugRenderResult::DebugRenderResult()
{
	this->renderLineArray = new std::vector<RenderLine>();
}

/*virtual*/ DebugRenderResult::~DebugRenderResult()
{
	delete this->renderLineArray;
}

void DebugRenderResult::AddRenderLine(const RenderLine& renderLine)
{
	this->renderLineArray->push_back(renderLine);
}

/*static*/ DebugRenderResult* DebugRenderResult::Create()
{
	return new DebugRenderResult();
}