#include "Level.h"
#include "Game.h"
#include "Scene.h"
#include "Hero.h"
#include "AssetCache.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Assets/CollisionShapeSet.h"
#include "Assets/LevelData.h"
#include <format>

using namespace Collision;

Level::Level()
{
	this->levelNumber = 0;
}

/*virtual*/ Level::~Level()
{
}

/*virtual*/ bool Level::Setup()
{
	std::string levelFile = std::format("Levels/Level{}.level", this->levelNumber);
	Reference<Asset> asset;
	if (!Game::Get()->GetAssetCache()->GrabAsset(levelFile, asset))
		return false;

	Reference<LevelData> levelData;
	levelData.SafeSet(asset.Get());
	if (!levelData)
		return false;

	for (const std::string& modelFile : levelData->modelFilesArray)
		Game::Get()->LoadAndPlaceRenderMesh(modelFile, Vector3(), Quaternion());

	Hero* hero = Game::Get()->SpawnEntity<Hero>();
	hero->SetRestartLocation(levelData->playerStartPosition);
	hero->SetRestartOrientation(levelData->playerStartOrientation);

	AxisAlignedBoundingBox physicsWorldBox;
	std::vector<Reference<CollisionShapeSet>> collisionShapeSetArray;
	for (const std::string& collisionFile : levelData->collisionFilesArray)
	{
		if (!Game::Get()->GetAssetCache()->GrabAsset(collisionFile, asset))
			return false;

		auto collisionShapeSet = dynamic_cast<CollisionShapeSet*>(asset.Get());
		if (!collisionShapeSet)
			return false;

		collisionShapeSetArray.push_back(collisionShapeSet);

		AxisAlignedBoundingBox boundingBox;
		if (!collisionShapeSet->GetBoundingBox(boundingBox))
			return false;

		physicsWorldBox.Expand(boundingBox);
	}

	physicsWorldBox.Scale(1.5);
	if (!Game::Get()->GetCollisionSystem()->Initialize(physicsWorldBox))
		return false;

	for(auto collisionShapeSet : collisionShapeSetArray)
	{
		for (Shape* shape : collisionShapeSet->GetCollisionShapeArray())
			Game::Get()->GetCollisionSystem()->AddShape(shape, 0 /*COLL_SYS_ADD_FLAG_ALLOW_SPLIT*/);	// TODO: Figure out why splitting fails.

		collisionShapeSet->Clear(false);
	}

	// TODO: Add support for floating platforms--those that animate up and down or
	//       side-to-side.  This makes platforming more challenging as you have to
	//       time your jumps from one place to another.

	return true;
}

/*virtual*/ bool Level::Shutdown(bool gameShuttingDown)
{
	Game::Get()->GetCollisionSystem()->Clear();
	Game::Get()->GetCollisionSystem()->Shutdown();
	Game::Get()->GetScene()->Clear();

	if (!gameShuttingDown)
	{
		// The end of one level marks the beginning of a new one; that is,
		// unless the whole game is shutting down.
		Level* nextLevel = Game::Get()->SpawnEntity<Level>();
		nextLevel->SetLevelNumber(this->GetLevelNumber() + 1);
	}

	return true;
}

/*virtual*/ bool Level::Tick(double deltaTime)
{
	// TODO: Animate floating platforms here.  Don't forgot to move the collision as well.
	return true;
}