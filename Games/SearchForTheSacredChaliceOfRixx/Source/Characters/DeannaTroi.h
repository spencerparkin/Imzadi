#pragma once

#include "Entities/Biped.h"
#include "Entities/TriggerBox.h"
#include "Action.h"
#include "RenderObjects/TextRenderObject.h"

class DeannaTroi : public Imzadi::Biped
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

	class LabeledAction : public Imzadi::Action
	{
	public:
		LabeledAction(DeannaTroi* troi);
		virtual ~LabeledAction();

		virtual void Init() override;
		virtual void Deinit() override;
		virtual void Tick(double deltaTime) override;
		virtual std::string GetActionLabel() const = 0;

	private:
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

	// TODO: Add action for being able to collect an item and put it in your inventory.
	// TODO: Add action for initiating a conversation with another character.

	uint32_t cameraHandle;
	double maxMoveSpeed;
	Imzadi::EventListenerHandle triggerBoxListenerHandle;
	Imzadi::ActionManager actionManager;
};