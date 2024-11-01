#pragma once

#include "Character.h"
#include "Math/Random.h"

/**
 * These are just dumb baddies that can kill the main character if touched.
 * They're easy to avoid for now, but maybe I can make them more interesting later.
 * 
 * There is some concern here that parody enters upon copyright infringement.
 * It's not a borg, though, it's a borggy!
 */
class Borggy : public Character
{
public:
	Borggy();
	virtual ~Borggy();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual std::string GetAnimName(Imzadi::Biped::AnimType animType) override;
	virtual void ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule) override;
	virtual void IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime) override;
	virtual void AdjustFacingDirection(double deltaTime) override;
	virtual void OnBipedAbyssFalling() override;

public:
	bool assimulatedHuman;

private:
	void HandlePlatformRayCast(double deltaTime);
	void HandleAttackRayCast();

	enum Disposition
	{
		MEANDERING,
		ATTACKING
	};

	enum MeanderState
	{
		UNKNOWN,
		ROTATE_LEFT_UNTIL_READY,
		ROTATE_RIGHT_UNTIL_READY,
		MOVE_FORWARD_UNTIL_BORDER_HIT
	};

	Disposition disposition;
	MeanderState meanderState;
	double rotationTimeRemainding;
	double meanderingRotationRate;
	double meanderingMoveSpeed;
	double attackMoveSpeed;
	Imzadi::Random random;
	Imzadi::Collision::TaskID rayCastQueryTaskID;
	Imzadi::Collision::TaskID rayCastAttackQueryTaskID;
};