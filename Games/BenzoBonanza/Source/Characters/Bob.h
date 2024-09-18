#pragma once

#include "Character.h"
#include "EventSystem.h"
#include "DialogSystem.h"

/**
 * Bob is another character in the game.  Not sure yet what his deal is.
 */
class Bob : public Character
{
public:
	Bob();
	virtual ~Bob();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual std::string GetAnimName(Imzadi::Biped::AnimType animType) override;
	virtual void IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime) override;
	virtual void ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule) override;
	virtual Imzadi::Vector3 GetPlatformSpaceFacingDirection() const override;

private:
	void HandleConversationBoundaryEvent(const ConvoBoundaryEvent* event);
	void CalculateTalkTarget(const ConvoBoundaryEvent* event);

	enum Disposition
	{
		RUN_AROUND_LIKE_AN_IDIOT,
		STOP_AND_TALK
	};

	Disposition disposition;
	std::vector<Imzadi::Vector3> waypointLoopArray;
	int waypointTargetIndex;
	double runSpeed;
	Imzadi::Vector3 talkTarget;
};