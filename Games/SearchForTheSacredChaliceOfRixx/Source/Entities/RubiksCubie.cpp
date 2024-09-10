#include "RubiksCubie.h"
#include "Entities/RubiksCubeMaster.h"
#include "Assets/RubiksCubieData.h"

RubiksCubie::RubiksCubie()
{
	this->masterHandle = 0;
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

	this->targetCubieToPuzzle = cubieData->GetCubieToPuzzleTransform();
	this->currentCubieToPuzzle = this->targetCubieToPuzzle;

	return true;
}

/*virtual*/ bool RubiksCubie::Shutdown()
{
	MovingPlatform::Shutdown();

	return true;
}

/*virtual*/ bool RubiksCubie::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (tickPass == Imzadi::TickPass::MOVE_UNCONSTRAINTED)
	{
		Imzadi::Reference<Imzadi::ReferenceCounted> ref;
		if (Imzadi::HandleManager::Get()->GetObjectFromHandle(this->masterHandle, ref))
		{
			auto master = dynamic_cast<RubiksCubeMaster*>(ref.Get());
			if (master)
			{
				const Imzadi::Transform& puzzleToWorld = master->GetPuzzleToWorldTransform();
				Imzadi::Transform cubieToWorld = puzzleToWorld * this->currentCubieToPuzzle;
				this->renderMesh->SetObjectToWorldTransform(cubieToWorld);
			}
		}
	}

	return true;
}