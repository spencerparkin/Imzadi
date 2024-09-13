#pragma once

#include "Character.h"
#include "EventSystem.h"
#include "DialogSystem.h"

/**
 * Command Riker is Deann's love interest, or at one point this was the case.
 */
class Riker : public Character
{
public:
	Riker();
	virtual ~Riker();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual std::string GetAnimName(Imzadi::Biped::AnimType animType) override;
	virtual void IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime) override;
	virtual void ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule) override;

private:
	void HandleConversationBoundaryEvent(const ConvoBoundaryEvent* event);

	enum Disposition
	{
		RUN_AROUND_LIKE_AN_IDIOT,
		STOP_AND_TALK
	};

	Disposition disposition;
	std::vector<Imzadi::Vector3> waypointLoopArray;
	int waypointTargetIndex;
	double runSpeed;
};