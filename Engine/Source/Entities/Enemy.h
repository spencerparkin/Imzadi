#pragma once

#include "PhysicsEntity.h"

namespace Imzadi
{
	/**
	 * These are the antagonists of our game saga.
	 */
	class IMZADI_API Enemy : public PhysicsEntity
	{
	public:
		Enemy();
		virtual ~Enemy();

		// TODO: Path following?
	};
}