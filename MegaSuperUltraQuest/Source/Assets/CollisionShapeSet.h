#pragma once

#include "AssetCache.h"
#include "Shape.h"
#include <vector>

/**
 * This is a set of collision shapes owned by the main thread.
 * When put into use, clones of them may be passed to the collision thread,
 * because the collision thread wants to take ownership of the shape memory.
 * This is analogous to the relationship between a RenderMesh asset and a
 * RenderMeshInstance object.
 */
class CollisionShapeSet : public Asset
{
public:
	CollisionShapeSet();
	virtual ~CollisionShapeSet();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;

	void Clear(bool deleteShapes);

	const std::vector<Collision::Shape*>& GetCollisionShapeArray() { return this->collisionShapeArray; }

private:
	std::vector<Collision::Shape*> collisionShapeArray;
};