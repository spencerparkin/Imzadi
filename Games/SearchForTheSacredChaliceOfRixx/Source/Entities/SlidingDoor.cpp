#include "SlidingDoor.h"
#include "Assets/SlidingDoorData.h"
#include "GameApp.h"

SlidingDoor::SlidingDoor()
{
	this->isOpen = false;
	this->isLocked = false;
}

/*virtual*/ SlidingDoor::~SlidingDoor()
{
}

/*virtual*/ bool SlidingDoor::Setup()
{
	if (!MovingPlatform::Setup())
		return false;

	auto doorData = dynamic_cast<SlidingDoorData*>(this->data.Get());
	if (!doorData)
		return false;

	this->isLocked = doorData->IsInitiallyLocked();

	Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener(
		doorData->GetDoorChannel(),
		Imzadi::EventListenerType::TRANSITORY,
		new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
			this->HandleDoorEvent(event);
		}));

	return true;
}

/*virtual*/ bool SlidingDoor::Shutdown()
{
	MovingPlatform::Shutdown();

	return true;
}

void SlidingDoor::HandleDoorEvent(const Imzadi::Event* event)
{
	if (event->GetName() == "OpenDoor")
	{
		if (this->isLocked)
		{
			auto game = (GameApp*)Imzadi::Game::Get();
			GameProgress* gameProgress = game->GetGameProgress();
			uint32_t keyCount = gameProgress->GetPossessedItemCount("key");
			if (keyCount > 0)
			{
				gameProgress->SetPossessedItemCount("key", keyCount - 1);
				this->isLocked = false;
			}
		}

		if (!this->isLocked)
			this->isOpen = true;
	}
}

/*virtual*/ bool SlidingDoor::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (this->isOpen)
	{
		if (!MovingPlatform::Tick(tickPass, deltaTime))
			return false;
	}

	return true;
}