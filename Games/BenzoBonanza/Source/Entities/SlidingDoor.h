#pragma once

#include "Entities/MovingPlatform.h"
#include "EventSystem.h"

/**
 * These are simply moving platforms that don't move until
 * signaled to do so.  The signal could be anything, and the
 * door could be configured to respond to this signal only if
 * it can reduce the player's possessed key-count by one.
 * These may, for example, be used in conjunction with a
 * trigger box that knows what signal/message to send and
 * on what channel.
 */
class SlidingDoor : public Imzadi::MovingPlatform
{
public:
	SlidingDoor();
	virtual ~SlidingDoor();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

private:
	void HandleDoorEvent(const Imzadi::Event* event);

	bool isOpen;
	bool isLocked;
};