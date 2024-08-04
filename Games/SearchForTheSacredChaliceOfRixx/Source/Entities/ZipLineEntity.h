#pragma once

#include "Entity.h"
#include "Assets/ZipLine.h"
#include "Collision/Query.h"
#include "Collision/Result.h"

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
	void HandleCollisionResult(Imzadi::CollisionQueryResult* collisionResult, int i);

	Imzadi::Reference<ZipLine> zipLine;
	Imzadi::TaskID collisionQueryTaskID[2];
	Imzadi::ShapeID sphereShapeID[2];
};