#include "RubiksCubie.h"
#include "Entities/RubiksCubeMaster.h"
#include "Assets/RubiksCubieData.h"
#include "GameApp.h"

//------------------------------ RubiksCubie ------------------------------

RubiksCubie::RubiksCubie()
{
	this->masterHandle = 0;
	this->eventHandle = 0;
	this->animationCurrentAngle = 0.0;
	this->animationTargetAngle = 0.0;
	this->animationRate = 3.0;
	this->animating = false;
	this->disposition = Disposition::FOLLOW_MASTER;
}

/*virtual*/ RubiksCubie::~RubiksCubie()
{
}

/*virtual*/ bool RubiksCubie::Setup()
{
	if (!MovingPlatform::Setup())
		return false;

	// Save this transform off for later use.  This is where the platform should
	// be placed once the puzzle has been solved by the player.
	this->platformObjectToWorld = this->renderMesh->GetObjectToWorldTransform();

	auto cubieData = dynamic_cast<RubiksCubieData*>(this->data.Get());
	if (!cubieData)
		return false;

	const std::string& masterName = cubieData->GetMasterName();
	Imzadi::Reference<Imzadi::Entity> foundEntity;
	if (Imzadi::Game::Get()->FindEntityByName(masterName, foundEntity))
		this->masterHandle = foundEntity->GetHandle();
	else
	{
		auto master = Imzadi::Game::Get()->SpawnEntity<RubiksCubeMaster>();
		master->SetName(masterName);
		master->SetPuzzleChannelName(cubieData->GetPuzzleChannelName());
		this->masterHandle = master->GetHandle();
	}

	this->solvedCubieToPuzzle = cubieData->GetCubieToPuzzleTransform();
	this->currentCubieToPuzzle = this->solvedCubieToPuzzle;

	this->eventHandle = Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener(
		cubieData->GetPuzzleChannelName(),
		Imzadi::EventListenerType::TRANSITORY,
		new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
			this->HandleCubieEvent(event);
		}));

	if (((GameApp*)Imzadi::Game::Get())->GetGameProgress()->WasMileStoneReached("rubiks_cube_solved"))
		this->disposition = Disposition::MOVE_TO_PLATFORMING_POSITION;

	return true;
}

/*virtual*/ bool RubiksCubie::Shutdown()
{
	MovingPlatform::Shutdown();

	// Don't let the event system hang on to a lambda with a capture value that is about to go out of scope.
	Imzadi::Game::Get()->GetEventSystem()->UnregisterEventListener(this->eventHandle);
	this->eventHandle = 0;

	return true;
}

/*virtual*/ bool RubiksCubie::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (tickPass == Imzadi::TickPass::MOVE_UNCONSTRAINTED)
	{
		if (this->disposition == Disposition::FOLLOW_MASTER)
		{
			Imzadi::Transform cubieToPuzzle;

			if (!this->animating)
				cubieToPuzzle = this->currentCubieToPuzzle;
			else
			{
				double animationAngleDelta = this->animationRate * deltaTime;
				bool animationComplete = false;

				if (this->animationCurrentAngle < this->animationTargetAngle)
				{
					this->animationCurrentAngle += animationAngleDelta;
					if (this->animationCurrentAngle >= this->animationTargetAngle)
					{
						this->animationCurrentAngle = this->animationTargetAngle;
						animationComplete = true;
					}
				}
				else if (this->animationCurrentAngle > this->animationTargetAngle)
				{
					this->animationCurrentAngle -= animationAngleDelta;
					if (this->animationCurrentAngle <= this->animationTargetAngle)
					{
						this->animationCurrentAngle = this->animationTargetAngle;
						animationComplete = true;
					}
				}
				else
					animationComplete = true;

				Imzadi::Transform rotation;
				rotation.matrix.SetFromAxisAngle(this->animationAxis, this->animationCurrentAngle);
				cubieToPuzzle = rotation * this->currentCubieToPuzzle;

				if (animationComplete)
				{
					this->currentCubieToPuzzle = cubieToPuzzle;
					this->animating = false;
				}
			}

			Imzadi::Reference<Imzadi::ReferenceCounted> ref;
			if (Imzadi::HandleManager::Get()->GetObjectFromHandle(this->masterHandle, ref))
			{
				auto master = dynamic_cast<RubiksCubeMaster*>(ref.Get());
				if (master)
				{
					Imzadi::Transform currentCubieToWorld = this->renderMesh->GetObjectToWorldTransform();
					const Imzadi::Transform& puzzleToWorld = master->GetPuzzleToWorldTransform();
					Imzadi::Transform cubieToWorld = puzzleToWorld * cubieToPuzzle;
					if (cubieToWorld != currentCubieToWorld)
					{
						this->renderMesh->SetObjectToWorldTransform(cubieToWorld);
						this->UpdateCollisionTransforms();
					}
				}
			}
		}
		else if (this->disposition == Disposition::MOVE_TO_PLATFORMING_POSITION)
		{
			Imzadi::Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
			static double translationRate = 10.0;
			static double rotationRate = 0.5;
			double translationStep = translationRate * deltaTime;
			double rotationStep = rotationRate * deltaTime;
			if (objectToWorld.MoveTo(objectToWorld, this->platformObjectToWorld, translationStep, rotationStep))
			{
				this->renderMesh->SetObjectToWorldTransform(objectToWorld);
				this->UpdateCollisionTransforms();
			}
		}
	}

	return true;
}

void RubiksCubie::HandleCubieEvent(const Imzadi::Event* event)
{
	auto cubieEvent = dynamic_cast<const RubiksCubieEvent*>(event);
	if (cubieEvent)
	{
		if (this->animating)
		{
			this->CompleteAnimationNow();
			this->animating = false;
		}

		if (cubieEvent->cutPlane.GetSide(this->currentCubieToPuzzle.translation) == Imzadi::Plane::Side::FRONT)
		{
			this->animationAxis = cubieEvent->cutPlane.unitNormal;
			this->animationCurrentAngle = 0.0;

			if (cubieEvent->rotation == RubiksCubieEvent::Rotation::CCW)
				this->animationTargetAngle = M_PI / 2.0;
			else
				this->animationTargetAngle = -M_PI / 2.0;

			if (cubieEvent->animate)
				this->animating = true;
			else
				this->CompleteAnimationNow();
		}
	}
	else if (event->GetName() == "CubiesDisperse")
	{
		this->disposition = Disposition::MOVE_TO_PLATFORMING_POSITION;
	}
}

void RubiksCubie::CompleteAnimationNow()
{
	Imzadi::Transform rotation;
	rotation.matrix.SetFromAxisAngle(this->animationAxis, this->animationTargetAngle);
	this->currentCubieToPuzzle = rotation * this->currentCubieToPuzzle;
}

bool RubiksCubie::IsSolved() const
{
	Imzadi::Matrix3x3 matrixA;
	matrixA.SetSnapped(this->currentCubieToPuzzle.matrix);

	Imzadi::Matrix3x3 matrixB;
	matrixB.SetSnapped(this->solvedCubieToPuzzle.matrix);

	if (matrixA != matrixB)
		return false;

	double epsilon = 1e-5;
	if (!this->solvedCubieToPuzzle.translation.IsPoint(this->currentCubieToPuzzle.translation, epsilon))
		return false;

	return true;
}

//------------------------------ RubiksCubieEvent ------------------------------

RubiksCubieEvent::RubiksCubieEvent()
{
	this->rotation = Rotation::CCW;
	this->animate = false;
}

RubiksCubieEvent::RubiksCubieEvent(const Imzadi::Plane& cutPlane, Rotation rotation, bool animate)
{
	this->cutPlane = cutPlane;
	this->rotation = rotation;
	this->animate = animate;
}

/*virtual*/ RubiksCubieEvent::~RubiksCubieEvent()
{
}