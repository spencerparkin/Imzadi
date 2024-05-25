#pragma once

#include "Entity.h"

/**
 * These are the antagonists of our game saga.
 */
class Enemy : public Entity
{
public:
	Enemy();
	virtual ~Enemy();

	// TODO: Path following?
};