#pragma once

#include "Character.h"
#include "Entities/TriggerBox.h"
#include "Action.h"
#include "RenderObjects/TextRenderObject.h"
#include "Collision/Query.h"
#include "Entities/FreeCam.h"

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
	virtual void Reset() override;

private:
	void HandleTriggerBoxEvent(const Imzadi::TriggerBoxEvent* event);
#if defined _DEBUG
	void HandleFreeCamEvent(const Imzadi::Event* event);
#endif

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
		std::string sceneObjectName;
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

	// TODO: Add action for being able to collect an item and put it in your inventory.

	uint32_t cameraHandle;
	double maxMoveSpeed;
	Imzadi::EventListenerHandle triggerBoxListenerHandle;
	Imzadi::EventListenerHandle freeCamListenerHandle;
	Imzadi::ActionManager actionManager;
	Imzadi::TaskID rayCastQueryTaskID;
};