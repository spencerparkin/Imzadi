#include "Result.h"
#include "Math/AxisAlignedBoundingBox.h"

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

void DebugRenderResult::AddLinesForBox(const AxisAlignedBoundingBox& box, const Vector3& color)
{
	RenderLine renderLine;
	renderLine.color = color;

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.minCorner.y, box.minCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.maxCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.minCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.maxCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.minCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.maxCorner.y, box.maxCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.minCorner.x, box.maxCorner.y, box.minCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.maxCorner.z);
	renderLine.line.point[1] = Vector3(box.minCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.maxCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.minCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.maxCorner.x, box.minCorner.y, box.maxCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.minCorner.x, box.minCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.maxCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.minCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.maxCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.minCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.maxCorner.x, box.maxCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);
}

//-------------------------------- RayCastResult --------------------------------

RayCastResult::RayCastResult()
{
	this->hitData.shapeID = 0;
}

/*virtual*/ RayCastResult::~RayCastResult()
{
}

/*static*/ RayCastResult* RayCastResult::Create()
{
	return new RayCastResult();
}