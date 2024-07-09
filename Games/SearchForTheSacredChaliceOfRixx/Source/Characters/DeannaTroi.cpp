#include "DeannaTroi.h"
#include "GameApp.h"
#include "Entities/FollowCam.h"
#include "EventSystem.h"
#include "Log.h"

DeannaTroi::DeannaTroi()
{
	this->cameraHandle = 0;
	this->maxMoveSpeed = 20.0;
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

	auto followCam = Imzadi::Game::Get()->SpawnEntity<Imzadi::FollowCam>();
	followCam->SetSubject(this);
	followCam->SetCameraUser("DeannaTroi");
	followCam->SetCamera(Imzadi::Game::Get()->GetCamera());

	this->cameraHandle = followCam->GetHandle();

	Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener(new Imzadi::LambdaEventListener(IMZADI_EVENT_FLAG_TRIGGER_BOX, [=](const Imzadi::Event* event) {
		this->HandleTriggerBoxEvent((const Imzadi::TriggerBoxEvent*)event);
	}));

	return true;
}

/*virtual*/ bool DeannaTroi::Shutdown()
{
	Biped::Shutdown();

	Imzadi::Game::Get()->PopControllerUser();

	return true;
}

void DeannaTroi::HandleTriggerBoxEvent(const Imzadi::TriggerBoxEvent* event)
{
	const std::string& triggerBoxName = event->GetName();
	switch (event->type)
	{
		case Imzadi::TriggerBoxEvent::Type::SHAPE_ENTERED:
		{
			IMZADI_LOG_INFO("Entered trigger box %s.", triggerBoxName.c_str());
			break;
		}
		case Imzadi::TriggerBoxEvent::Type::SHAPE_EXITED:
		{
			IMZADI_LOG_INFO("Exited trigger box %s.", triggerBoxName.c_str());
			break;
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

	//...

	return true;
}

/*virtual*/ void DeannaTroi::Reset()
{
	// TODO: This is where we might incur a penalty for dying.

	Biped::Reset();
}