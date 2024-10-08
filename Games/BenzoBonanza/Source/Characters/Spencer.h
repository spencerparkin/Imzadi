#pragma once

#include "Character.h"
#include "DialogSystem.h"

/**
 * This is me.
 */
class Spencer : public Character
{
public:
	Spencer();
	virtual ~Spencer();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual void IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime) override;
	virtual std::string GetAnimName(Imzadi::Biped::AnimType animType) override;
	virtual void AdjustFacingDirection(double deltaTime) override;
	virtual void ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule) override;
	virtual bool OnBipedDied() override;

private:
	void HandleGeneralEvent(const Imzadi::Event* event);
	void HandleConversationBoundaryEvent(const ConvoBoundaryEvent* event);
	void CelebrateIfAllMedsReturned();

	enum Disposition
	{
		COLLECT_BENZOS_FROM_PLAYER,
		COMMIT_SUICIDE,
		CELEBRATE
	};

	Disposition disposition;
	Imzadi::Vector3 runDirection;
	double runSpeed;
};