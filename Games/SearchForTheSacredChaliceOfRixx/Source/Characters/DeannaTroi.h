#pragma once

#include "Character.h"
#include "Entities/TriggerBox.h"
#include "Action.h"
#include "RenderObjects/TextRenderObject.h"
#include "Collision/Query.h"
#include "Entities/FreeCam.h"

#define MAX_PLATFORM_LANDING_SPEED		60.0

class Pickup;

class DeannaTroi : public Character
{
public:
	DeannaTroi();
	virtual ~DeannaTroi();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual void AccumulateForces(Imzadi::Vector3& netForce) override;
	virtual void IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime) override;
	virtual bool ConstraintVelocityWithGround() override;
	virtual void Reset() override;
	virtual bool HangingOnToZipLine() override;
	virtual std::string GetZipLineAnimationName() override;
	virtual std::string GetAnimName(Imzadi::Biped::AnimType animType) override;
	virtual bool OnBipedDied() override;
	virtual void OnBipedFatalLanding() override;
	virtual void OnBipedAbyssFalling() override;
	virtual void OnBipedBaddyHit() override;
	virtual void ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule) override;

private:
	void HandleTriggerBoxEvent(const Imzadi::TriggerBoxEvent* event);
#if defined _DEBUG
	void HandleFreeCamEvent(const Imzadi::Event* event);
#endif
	void HandleEntityOverlapResults();

	class LabeledAction : public Imzadi::Action
	{
	public:
		LabeledAction(DeannaTroi* troi);
		virtual ~LabeledAction();

		virtual void Init() override;
		virtual void Deinit() override;
		virtual void Tick(double deltaTime) override;
		virtual std::string GetActionLabel() const = 0;

	protected:
		void UpdateTransform();

		uint32_t entityHandle;
		Imzadi::Reference<Imzadi::TextRenderObject> textRenderObject;
	};

	class TeleportToLevelAction : public LabeledAction
	{
	public:
		TeleportToLevelAction(DeannaTroi* troi);
		virtual ~TeleportToLevelAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		std::string targetLevel;
	};

	class TalkToEntityAction : public LabeledAction
	{
	public:
		TalkToEntityAction(DeannaTroi* troi);
		virtual ~TalkToEntityAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		std::string targetEntity;
	};

	class CollectPickupAction : public LabeledAction
	{
	public:
		CollectPickupAction(DeannaTroi* troi);
		virtual ~CollectPickupAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		Imzadi::Reference<Pickup> pickup;
	};

	class ControlRubiksCubeAction : public LabeledAction
	{
	public:
		ControlRubiksCubeAction(DeannaTroi* troi);
		virtual ~ControlRubiksCubeAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		std::string masterName;
	};

	uint32_t cameraHandle;
	double maxMoveSpeed;
	Imzadi::EventListenerHandle triggerBoxListenerHandle;
	Imzadi::EventListenerHandle freeCamListenerHandle;
	Imzadi::ActionManager actionManager;
	Imzadi::Collision::TaskID rayCastQueryTaskID;
	Imzadi::Collision::TaskID entityOverlapQueryTaskID;
};