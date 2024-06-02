#pragma once

#include "Entity.h"
#include "Shape.h"
#include "RenderObjects/RenderMeshInstance.h"

class MovingPlatformData;
class CollisionShapeSet;

/**
 * These are element of the level that make platforming a bit more challenging and fun.
 */
class MovingPlatform : public Entity
{
public:
	MovingPlatform();
	virtual ~MovingPlatform();

	/**
	 * Figure out how the platform will move.
	 */
	virtual bool Setup() override;

	/**
	 * Clean-up.
	 */
	virtual bool Shutdown(bool gameShuttingDown) override;

	/**
	 * Animate the platform's movement.
	 */
	virtual bool Tick(double deltaTime) override;

	/**
	 * Specify where this platform's configuration data is on disk.
	 */
	void SetMovingPlatformFile(const std::string& file) { this->movingPlatformFile = file; }

private:
	Reference<MovingPlatformData> data;
	Reference<RenderMeshInstance> renderMesh;
	std::vector<Collision::ShapeID> collisionShapeArray;
	std::string movingPlatformFile;
	int targetDeltaIndex;
	int bounceDelta;
};