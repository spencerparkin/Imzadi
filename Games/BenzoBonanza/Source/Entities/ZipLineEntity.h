#pragma once

#include "Entity.h"
#include "Assets/ZipLine.h"
#include "Collision/Query.h"
#include "Collision/Result.h"

class ZipLineRider;
class Character;

class ZipLineEntity : public Imzadi::Entity
{
public:
	ZipLineEntity();
	virtual ~ZipLineEntity();

	void SetZipLine(ZipLine* givenZipLine);

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

private:
	void HandleCollisionResult(Imzadi::Collision::CollisionQueryResult* collisionResult);
	ZipLineRider* FindZipLineRider(Character* character);

	Imzadi::Reference<ZipLine> zipLine;
	Imzadi::Collision::TaskID collisionQueryTaskID;
	Imzadi::Collision::ShapeID sphereShapeID;
};