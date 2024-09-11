#include "RubiksCubeMaster.h"
#include "Entities/RubiksCubie.h"
#include "Game.h"
#include "Camera.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "Assets/RenderMesh.h"
#include "Math/Random.h"

RubiksCubeMaster::RubiksCubeMaster()
{
	this->state = State::OFF;
	this->cameraTransitionRate = 0.5;
	this->cameraTransitionAlpha = 0.0;
	this->rotationSensativity = 2.0;
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

	// We should be setup *after* all the cubies are setup, so I think this should work.
#if defined _DEBUG
	int numScrambleTwists = 1;		// I have spent time solving the cube in game by hand, but this is needed for debugging purposes.
#else
	int numScrambleTwists = 50;		// I spent about 10 minutes solving the cube in game by hand, and it's not so bad, really.
#endif
	Imzadi::Random random;
	random.SetSeedUsingTime();
	for (int i = 0; i < numScrambleTwists; i++)
	{
		Imzadi::Plane cutPlane;

		switch (random.InRange(0, 2))
		{
			case 0: cutPlane.unitNormal.SetComponents(1.0, 0.0, 0.0); break;
			case 1: cutPlane.unitNormal.SetComponents(0.0, 1.0, 0.0); break;
			case 2: cutPlane.unitNormal.SetComponents(0.0, 0.0, 1.0); break;
		}

		if (random.CoinFlip())
			cutPlane.unitNormal = -cutPlane.unitNormal;

		cutPlane.center = cutPlane.unitNormal * 5.0;	// The cubies are 10x10x10.

		auto event = new RubiksCubieEvent();
		event->animate = false;
		event->rotation = random.CoinFlip() ? RubiksCubieEvent::Rotation::CCW : RubiksCubieEvent::CW;
		event->cutPlane = cutPlane;
		Imzadi::Game::Get()->GetEventSystem()->SendEvent(this->puzzleChannelName, event);
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
					if (this->IsCubeSolved())
						Imzadi::Game::Get()->GetEventSystem()->SendEvent(this->puzzleChannelName, new Imzadi::Event("CubiesDisperse"));
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
				if (controller->ButtonPressed(Imzadi::Button::BACK))
					this->Enable(false);

				Imzadi::Camera* camera = Imzadi::Game::Get()->GetCamera();
				const Imzadi::Transform& cameraToWorld = camera->GetCameraToWorldTransform();

				Imzadi::Vector3 xAxis, yAxis, zAxis;
				cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

				Imzadi::Vector2 joyStickVector = controller->GetAnalogJoyStick(Imzadi::Button::L_JOY_STICK);

				double angleX = -joyStickVector.y * this->rotationSensativity * deltaTime;
				double angleY = joyStickVector.x * this->rotationSensativity * deltaTime;

				Imzadi::Matrix3x3 rotationX, rotationY;
				rotationX.SetFromAxisAngle(xAxis, angleX);
				rotationY.SetFromAxisAngle(yAxis, angleY);

				this->puzzleToWorld.matrix = (rotationX * rotationY * this->puzzleToWorld.matrix).Orthonormalized(IMZADI_AXIS_FLAG_X);

				bool aButtonPressed = controller->ButtonPressed(Imzadi::Button::A_BUTTON);
				bool bButtonPressed = controller->ButtonPressed(Imzadi::Button::B_BUTTON);

				if (aButtonPressed || bButtonPressed)
				{
					Imzadi::Transform worldToPuzzle;
					worldToPuzzle.Invert(this->puzzleToWorld);
					
					Imzadi::Matrix3x3 matrix;
					matrix.SetSnapped(worldToPuzzle.matrix * cameraToWorld.matrix);

					Imzadi::Plane cutPlane;
					cutPlane.unitNormal = matrix.GetColumnVector(2);
					cutPlane.center = cutPlane.unitNormal * 5.0;	// The cubies are 10x10x10.

					auto event = new RubiksCubieEvent();
					event->animate = true;
					event->rotation = aButtonPressed ? RubiksCubieEvent::Rotation::CCW : RubiksCubieEvent::CW;
					event->cutPlane = cutPlane;
					Imzadi::Game::Get()->GetEventSystem()->SendEvent(this->puzzleChannelName, event);
				}
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
		this->targetCameraTransform.LookAt(this->puzzleToWorld.translation + Imzadi::Vector3(0.0, 180.0, -180.0), puzzleToWorld.translation, Imzadi::Vector3(0.0, 1.0, 0.0));
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

bool RubiksCubeMaster::IsCubeSolved()
{
	std::vector<RubiksCubie*> cubieArray;
	if (!Imzadi::Game::Get()->FindAllEntitiesOfType<RubiksCubie>(cubieArray))
		return false;

	int solvedCount = 0;
	for (const RubiksCubie* cubie : cubieArray)
		if (cubie->GetMasterHandle() == this->GetHandle() && cubie->IsSolved())
			solvedCount++;

	return solvedCount == 26;	// 3*3*3 - 1
}