#include "CollisionShapeSet.h"

using namespace Collision;

CollisionShapeSet::CollisionShapeSet()
{
}

/*virtual*/ CollisionShapeSet::~CollisionShapeSet()
{
	this->Clear(true);
}

/*virtual*/ bool CollisionShapeSet::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	// TODO: Load collision shapes here.  I'll need to write some Python script first to generate the asset files.
	return true;
}

/*virtual*/ bool CollisionShapeSet::Unload()
{
	this->Clear(true);

	return true;
}

void CollisionShapeSet::Clear(bool deleteShapes)
{
	if (deleteShapes)
		for (Shape* shape : this->collisionShapeArray)
			Shape::Free(shape);

	this->collisionShapeArray.clear();
}