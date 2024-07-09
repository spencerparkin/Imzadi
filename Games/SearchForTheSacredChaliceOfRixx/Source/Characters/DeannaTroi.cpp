#include "DeannaTroi.h"
#include "GameApp.h"
#include "Entities/FollowCam.h"
#include "EventSystem.h"
#include "Log.h"

//------------------------------------ DeannaTroi ------------------------------------

DeannaTroi::DeannaTroi()
{
	this->cameraHandle = 0;
	this->maxMoveSpeed = 20.0;
	this->triggerBoxListenerHandle = 0;
}

/*virtual*/ DeannaTroi::~DeannaTroi()
{
}

/*virtual*/ bool DeannaTroi::Setup()
{
	std::string modelFile = "Models/DeannaTroi/Troi.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile, &this->restartLocation, &this->restartOrientation));

	if (!Biped::Setup())
		return false;

	Imzadi::Game::Get()->PushControllerUser("DeannaTroi");
	this->actionManager.SetControllerUser("DeannaTroi");

	auto followCam = Imzadi::Game::Get()->SpawnEntity<Imzadi::FollowCam>();
	followCam->SetSubject(this);
	followCam->SetCameraUser("DeannaTroi");
	followCam->SetCamera(Imzadi::Game::Get()->GetCamera());

	this->cameraHandle = followCam->GetHandle();

	this->triggerBoxListenerHandle = Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("TriggerBox", new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
		this->HandleTriggerBoxEvent((const Imzadi::TriggerBoxEvent*)event);
	}));

	return true;
}

/*virtual*/ bool DeannaTroi::Shutdown()
{
	Biped::Shutdown();

	this->actionManager.Clear();

	Imzadi::Game::Get()->PopControllerUser();

	if (this->triggerBoxListenerHandle)
	{
		Imzadi::Game::Get()->GetEventSystem()->UnregisterEventListener(this->triggerBoxListenerHandle);
		this->triggerBoxListenerHandle = 0;
	}

	return true;
}

void DeannaTroi::HandleTriggerBoxEvent(const Imzadi::TriggerBoxEvent* event)
{
	if (this->collisionShapeID == event->shapeID)
	{
		const std::string& triggerBoxName = event->GetName();
		switch (event->type)
		{
			case Imzadi::TriggerBoxEvent::Type::SHAPE_ENTERED:
			{
				IMZADI_LOG_INFO("Entered trigger box \"%s\".", triggerBoxName.c_str());

				if (triggerBoxName.find("JumpTo") == 0)
				{
					auto action = new TeleportToLevelAction(this);
					action->targetLevel = triggerBoxName.substr(6);
					this->actionManager.BindAction(XINPUT_GAMEPAD_A, action);
				}

				break;
			}
			case Imzadi::TriggerBoxEvent::Type::SHAPE_EXITED:
			{
				IMZADI_LOG_INFO("Exited trigger box \"%s\".", triggerBoxName.c_str());

				if (triggerBoxName.find("JumpTo") == 0)
					this->actionManager.UnbindAction(XINPUT_GAMEPAD_A);

				break;
			}
		}
	}
}

/*virtual*/ void DeannaTroi::AccumulateForces(Imzadi::Vector3& netForce)
{
	PhysicsEntity::AccumulateForces(netForce);

	Imzadi::Controller* controller = Imzadi::Game::Get()->GetController("DeannaTroi");
	if (!controller)
		return;

	if (this->inContactWithGround && controller->ButtonPressed(XINPUT_GAMEPAD_Y))
	{
		Imzadi::Vector3 jumpForce(0.0, 1000.0, 0.0);
		netForce += jumpForce;
	}
}

/*virtual*/ void DeannaTroi::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	Biped::IntegrateVelocity(acceleration, deltaTime);

	if (this->inContactWithGround)
	{
		Imzadi::Reference<ReferenceCounted> followCamRef;
		Imzadi::HandleManager::Get()->GetObjectFromHandle(this->cameraHandle, followCamRef);
		auto followCam = dynamic_cast<Imzadi::FollowCam*>(followCamRef.Get());
		if (!followCam)
			return;

		Imzadi::Vector2 leftStick(0.0, 0.0);
		Imzadi::Controller* controller = Imzadi::Game::Get()->GetController("DeannaTroi");
		if (controller)
			controller->GetAnalogJoyStick(Imzadi::Controller::Side::LEFT, leftStick.x, leftStick.y);

		Imzadi::Camera* camera = followCam->GetCamera();
		if (!camera)
			return;

		Imzadi::Transform cameraToWorld = camera->GetCameraToWorldTransform();

		Imzadi::Vector3 xAxis, yAxis, zAxis;
		cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

		Imzadi::Vector3 upVector(0.0, 1.0, 0.0);
		xAxis = xAxis.RejectedFrom(upVector).Normalized();
		zAxis = zAxis.RejectedFrom(upVector).Normalized();

		Imzadi::Vector3 moveDelta = (xAxis * leftStick.x - zAxis * leftStick.y) * this->maxMoveSpeed;

		// We don't stomp the Y-component here, because we still want
		// the effects of gravity applied on our character continuously.
		this->velocity.x = moveDelta.x;
		this->velocity.z = moveDelta.z;
	}
}

/*virtual*/ bool DeannaTroi::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Biped::Tick(tickPass, deltaTime))
		return false;

	this->actionManager.Tick(deltaTime);

	return true;
}

/*virtual*/ void DeannaTroi::Reset()
{
	// TODO: This is where we might incur a penalty for dying.

	Biped::Reset();
}

//------------------------------------ DeannaTroi::LabeledAction ------------------------------------

DeannaTroi::LabeledAction::LabeledAction(DeannaTroi* troi)
{
	this->entityHandle = troi->GetHandle();
}

/*virtual*/ DeannaTroi::LabeledAction::~LabeledAction()
{
}

/*virtual*/ void DeannaTroi::LabeledAction::Init()
{
	uint32_t flags =
		Imzadi::TextRenderObject::Flag::ALWAYS_FACING_CAMERA |
		Imzadi::TextRenderObject::Flag::ALWAYS_ON_TOP |
		Imzadi::TextRenderObject::Flag::CENTER_JUSTIFY;

	this->textRenderObject = new Imzadi::TextRenderObject();
	this->textRenderObject->SetText(this->GetActionLabel());
	this->textRenderObject->SetFont("Roboto_Regular");
	this->textRenderObject->SetColor(Imzadi::Vector3(1.0, 0.0, 0.0));
	this->textRenderObject->SetFlags(flags);
	
	this->UpdateTransform();

	Imzadi::Scene* scene = Imzadi::Game::Get()->GetScene();
	this->sceneObjectName = scene->AddRenderObject(this->textRenderObject.Get());
}

/*virtual*/ void DeannaTroi::LabeledAction::Deinit()
{
	Imzadi::Scene* scene = Imzadi::Game::Get()->GetScene();
	scene->RemoveRenderObject(this->sceneObjectName);
	this->textRenderObject.Reset();
}

/*virtual*/ void DeannaTroi::LabeledAction::Tick(double deltaTime)
{
	this->textRenderObject->SetText(this->GetActionLabel());
	this->UpdateTransform();
}

void DeannaTroi::LabeledAction::UpdateTransform()
{
	Imzadi::Reference<ReferenceCounted> ref;
	if (Imzadi::HandleManager::Get()->GetObjectFromHandle(this->entityHandle, ref))
	{
		auto troi = dynamic_cast<DeannaTroi*>(ref.Get());
		if (troi)
		{
			Imzadi::Transform transform;
			troi->GetTransform(transform);
			transform.matrix.SetIdentity();
			transform.matrix.SetUniformScale(20.0);
			this->textRenderObject->SetTransform(transform);
		}
	}
}

//------------------------------------ DeannaTroi::TeleportToLevelAction ------------------------------------

DeannaTroi::TeleportToLevelAction::TeleportToLevelAction(DeannaTroi* troi) : LabeledAction(troi)
{
}

/*virtual*/ DeannaTroi::TeleportToLevelAction::~TeleportToLevelAction()
{
}

/*virtual*/ bool DeannaTroi::TeleportToLevelAction::Perform()
{
	Imzadi::EventSystem* eventSystem = Imzadi::Game::Get()->GetEventSystem();
	eventSystem->SendEvent("LevelTransition", new Imzadi::Event(this->targetLevel));
	return false;	// Return false here to unbind the action once it's been performed.
}

/*virtual*/ std::string DeannaTroi::TeleportToLevelAction::GetActionLabel() const
{
	return std::format("Press \"A\" to teleport to {}.", this->targetLevel.c_str());
}