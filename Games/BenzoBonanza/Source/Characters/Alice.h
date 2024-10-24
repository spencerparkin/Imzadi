#pragma once

#include "Character.h"
#include "Entities/TriggerBox.h"
#include "Action.h"
#include "RenderObjects/TextRenderObject.h"
#include "Assets/NavGraph.h"
#include "Collision/Query.h"
#include "Entities/FreeCam.h"

#define MAX_PLATFORM_LANDING_SPEED		60.0
//#define AUTHOR_NAV_GRAPH_CAPABILITY

class Pickup;

/**
 * Alice is the main character of the game and is controlled by the player.
 */
class Alice : public Character
{
public:
	Alice();
	virtual ~Alice();

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
	void HandleFreeCamEvent(const Imzadi::Event* event);
	void HandleEntityOverlapResults();

	class LabeledAction : public Imzadi::Action
	{
	public:
		LabeledAction(Alice* alice);
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
		TeleportToLevelAction(Alice* alice);
		virtual ~TeleportToLevelAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		std::string targetLevel;
	};

	class TalkToEntityAction : public LabeledAction
	{
	public:
		TalkToEntityAction(Alice* alice);
		virtual ~TalkToEntityAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		std::string targetEntity;
	};

	class CollectPickupAction : public LabeledAction
	{
	public:
		CollectPickupAction(Alice* alice);
		virtual ~CollectPickupAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		Imzadi::Reference<Pickup> pickup;
	};

	class ControlRubiksCubeAction : public LabeledAction
	{
	public:
		ControlRubiksCubeAction(Alice* alice);
		virtual ~ControlRubiksCubeAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		std::string masterName;
	};

	class OpenDoorAction : public LabeledAction
	{
	public:
		OpenDoorAction(Alice* alice);
		virtual ~OpenDoorAction();

		virtual bool Perform() override;
		virtual std::string GetActionLabel() const override;

		std::string doorChannel;
	};

	uint32_t cameraHandle;
	double maxMoveSpeed;
	Imzadi::EventListenerHandle triggerBoxListenerHandle;
	Imzadi::EventListenerHandle freeCamListenerHandle;
	Imzadi::ActionManager actionManager;
	Imzadi::Collision::TaskID rayCastQueryTaskID;
	Imzadi::Collision::TaskID entityOverlapQueryTaskID;
#if defined AUTHOR_NAV_GRAPH_CAPABILITY
	bool authoringNavGraph;
	Imzadi::Reference<Imzadi::NavGraph::Node> currentNode;
#endif //AUTHOR_NAV_GRAPH_CAPABILITY
};