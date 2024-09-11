#include "RubiksCubie.h"
#include "Entities/RubiksCubeMaster.h"
#include "Assets/RubiksCubieData.h"

//------------------------------ RubiksCubie ------------------------------

RubiksCubie::RubiksCubie()
{
	this->masterHandle = 0;
	this->eventHandle = 0;
	this->animationCurrentAngle = 0.0;
	this->animationTargetAngle = 0.0;
	this->animationRate = 3.0;
	this->animating = false;
}

/*virtual*/ RubiksCubie::~RubiksCubie()
{
}

/*virtual*/ bool RubiksCubie::Setup()
{
	if (!MovingPlatform::Setup())
		return false;

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
			this->HandleCubieEvent(dynamic_cast<const RubiksCubieEvent*>(event));
		}));

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
				const Imzadi::Transform& puzzleToWorld = master->GetPuzzleToWorldTransform();
				Imzadi::Transform cubieToWorld = puzzleToWorld * cubieToPuzzle;
				this->renderMesh->SetObjectToWorldTransform(cubieToWorld);
			}
		}
	}

	return true;
}

void RubiksCubie::HandleCubieEvent(const RubiksCubieEvent* event)
{
	if (this->animating)
	{
		this->CompleteAnimationNow();
		this->animating = false;
	}

	if (event->cutPlane.GetSide(this->currentCubieToPuzzle.translation) == Imzadi::Plane::Side::FRONT)
	{
		this->animationAxis = event->cutPlane.unitNormal;
		this->animationCurrentAngle = 0.0;

		if (event->rotation == RubiksCubieEvent::Rotation::CCW)
			this->animationTargetAngle = M_PI / 2.0;
		else
			this->animationTargetAngle = -M_PI / 2.0;

		if (event->animate)
			this->animating = true;
		else
			this->CompleteAnimationNow();
	}
}

void RubiksCubie::CompleteAnimationNow()
{
	Imzadi::Transform rotation;
	rotation.matrix.SetFromAxisAngle(this->animationAxis, this->animationTargetAngle);
	this->currentCubieToPuzzle = rotation * this->currentCubieToPuzzle;
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