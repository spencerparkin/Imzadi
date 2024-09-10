#include "RubiksCubeMaster.h"
#include "Game.h"
#include "Camera.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "Assets/RenderMesh.h"

RubiksCubeMaster::RubiksCubeMaster()
{
	this->state = State::OFF;
	this->cameraTransitionRate = 0.5;
	this->cameraTransitionAlpha = 0.0;
}

/*virtual*/ RubiksCubeMaster::~RubiksCubeMaster()
{
}

/*virtual*/ bool RubiksCubeMaster::Setup()
{
	Imzadi::Reference<Imzadi::RenderObject> renderObject;
	if (Imzadi::Game::Get()->GetScene()->FindRenderObject("CenterPlatform", renderObject))
	{
		auto renderMesh = dynamic_cast<Imzadi::RenderMeshInstance*>(renderObject.Get());
		if (renderMesh)
		{
			Imzadi::Transform portToObject;
			if (renderMesh->GetRenderMesh()->GetPort("Port1", portToObject))
			{
				this->puzzleToWorld = renderMesh->GetObjectToWorldTransform() * portToObject;
			}
		}
	}

	return true;
}

/*virtual*/ bool RubiksCubeMaster::Shutdown()
{
	return true;
}

/*virtual*/ bool RubiksCubeMaster::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (tickPass != Imzadi::TickPass::MOVE_UNCONSTRAINTED)
		return true;

	if (this->state == State::OFF)
		return true;

	switch (this->state)
	{
		case State::POWERING_UP:
		case State::POWERING_DOWN:
		{
			Imzadi::Camera* camera = Imzadi::Game::Get()->GetCamera();
			Imzadi::Transform cameraToWorld = camera->GetCameraToWorldTransform();
			this->cameraTransitionAlpha += this->cameraTransitionRate * deltaTime;
			if (this->cameraTransitionAlpha < 1.0)
				cameraToWorld.InterapolateBoneTransforms(this->sourceCameraTransform, this->targetCameraTransform, this->cameraTransitionAlpha);
			else
			{
				cameraToWorld = this->targetCameraTransform;

				if (this->state == State::POWERING_UP)
					this->state = State::OPERATING;
				else if (this->state == State::POWERING_DOWN)
				{
					this->state = State::OFF;
					Imzadi::Game::Get()->PopControllerUser();
				}
			}

			camera->SetCameraToWorldTransform(cameraToWorld);
			break;
		}
		case State::OPERATING:
		{
			Imzadi::Input* controller = Imzadi::Game::Get()->GetController("RubiksCubeMaster");
			if (controller)
			{
				if (controller->ButtonPressed(Imzadi::Button::B_BUTTON))
				{
					this->Enable(false);
				}

				// TODO: Joysticks to rotate the cube.  D-pad to rotate faces.
			}

			break;
		}
	}

	return true;
}

void RubiksCubeMaster::Enable(bool enable)
{
	if (this->state == State::OFF && enable)
	{
		this->state = State::POWERING_UP;

		Imzadi::Game::Get()->PushControllerUser("RubiksCubeMaster");

		Imzadi::Camera* camera = Imzadi::Game::Get()->GetCamera();
		this->originalCameraTransform = camera->GetCameraToWorldTransform();
		this->sourceCameraTransform = this->originalCameraTransform;
		this->targetCameraTransform.LookAt(this->puzzleToWorld.translation + Imzadi::Vector3(0.0, 170.0, -170.0), puzzleToWorld.translation, Imzadi::Vector3(0.0, 1.0, 0.0));
		this->cameraTransitionAlpha = 0.0;
	}
	else if (this->state == State::OPERATING && !enable)
	{
		this->state = State::POWERING_DOWN;

		Imzadi::Camera* camera = Imzadi::Game::Get()->GetCamera();
		this->sourceCameraTransform = camera->GetCameraToWorldTransform();
		this->targetCameraTransform = this->originalCameraTransform;
		this->cameraTransitionAlpha = 0.0;
	}
}