#pragma once

#include "PhysicsEntity.h"

/**
 * These are the antagonists of our game saga.
 */
class Enemy : public PhysicsEntity
{
public:
	Enemy();
	virtual ~Enemy();

	// TODO: Path following?
};