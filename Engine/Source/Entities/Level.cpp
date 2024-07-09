#include "Level.h"
#include "Game.h"
#include "Scene.h"
#include "Biped.h"
#include "AssetCache.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Assets/CollisionShapeSet.h"
#include "Assets/LevelData.h"
#include "RenderObjects/SkyDomeRenderObject.h"
#include "MovingPlatform.h"
#include "TriggerBox.h"
#include "Log.h"
#include <format>

using namespace Imzadi;

Level::Level()
{
	this->levelName = "?";
}

/*virtual*/ Level::~Level()
{
}

/*virtual*/ Biped* Level::SpawnMainCharacter()
{
	return nullptr;
}

/*virtual*/ bool Level::Setup()
{
	std::string levelFile = std::format("Levels/{}.level", this->levelName.c_str());
	Reference<Asset> asset;
	if (!Game::Get()->GetAssetCache()->LoadAsset(levelFile, asset))
		return false;

	Reference<LevelData> levelData;
	levelData.SafeSet(asset.Get());
	if (!levelData)
		return false;

	for (const std::string& modelFile : levelData->GetModelFilesArray())
		Game::Get()->LoadAndPlaceRenderMesh(modelFile);

	std::string skyDomeFile = levelData->GetSkyDomeFile();
	if (skyDomeFile.length() > 0)
	{
		Reference<RenderObject> renderObject = Game::Get()->LoadAndPlaceRenderMesh(skyDomeFile);

		std::string cubeTextureFile = levelData->GetCubeTextureFile();
		if (cubeTextureFile.length() > 0)
		{
			if (Game::Get()->GetAssetCache()->LoadAsset(cubeTextureFile, asset))
			{
				CubeTexture* cubeTexture = dynamic_cast<CubeTexture*>(asset.Get());
				if (cubeTexture)
				{
					auto skyDomeRenderObj = dynamic_cast<SkyDomeRenderObject*>(renderObject.Get());
					if (skyDomeRenderObj)
						skyDomeRenderObj->GetSkyDome()->SetCubeTexture(cubeTexture);
				}
			}
		}
	}

	Biped* biped = this->SpawnMainCharacter();
	if (biped)
	{
		biped->SetRestartLocation(levelData->GetPlayerStartPosition());
		biped->SetRestartOrientation(levelData->GetPlayerStartOrientation());
	}

	// TODO: Spawn other characters throughout the level as dictated in the level JSON file.

	AxisAlignedBoundingBox collisionWorldBox;
	std::vector<Reference<CollisionShapeSet>> collisionShapeSetArray;
	for (const std::string& collisionFile : levelData->GetCollisionFilesArray())
	{
		if (!Game::Get()->GetAssetCache()->LoadAsset(collisionFile, asset))
			return false;

		auto collisionShapeSet = dynamic_cast<CollisionShapeSet*>(asset.Get());
		if (!collisionShapeSet)
			return false;

		collisionShapeSetArray.push_back(collisionShapeSet);

		AxisAlignedBoundingBox boundingBox;
		if (!collisionShapeSet->GetBoundingBox(boundingBox))
			return false;

		collisionWorldBox.Expand(boundingBox);
	}

	collisionWorldBox.Scale(1.5);
	collisionWorldBox.Scale(1.0, 3.0, 1.0);

	if (!Game::Get()->GetCollisionSystem()->Initialize(collisionWorldBox))
		return false;

	for(auto collisionShapeSet : collisionShapeSetArray)
	{
		for (Shape* shape : collisionShapeSet->GetCollisionShapeArray())
			Game::Get()->GetCollisionSystem()->AddShape(shape, 0 /*IMZADI_ADD_FLAG_ALLOW_SPLIT*/);	// TODO: Figure out why splitting fails.

		collisionShapeSet->Clear(false);
	}

	for (const std::string& movingPlatformFile : levelData->GetMovingPlatformFilesArray())
	{
		auto movingPlatform = Game::Get()->SpawnEntity<MovingPlatform>();
		movingPlatform->SetMovingPlatformFile(movingPlatformFile);
	}

	for (const std::string& triggerBoxFile : levelData->GetTriggerBoxFilesArray())
	{
		if (!Game::Get()->GetAssetCache()->LoadAsset(triggerBoxFile, asset))
		{
			IMZADI_LOG_ERROR("Failed to load trigger box file: %s", triggerBoxFile.c_str());
			return false;
		}

		Reference<TriggerBoxData> triggerBoxData;
		triggerBoxData.SafeSet(asset.Get());
		if (!triggerBoxData)
		{
			IMZADI_LOG_ERROR("Whatever loaded for the trigger box data wasn't trigger box data.");
			return false;
		}

		auto triggerBox = Game::Get()->SpawnEntity<TriggerBox>();
		triggerBox->SetData(triggerBoxData);
	}

	return true;
}

/*virtual*/ bool Level::Shutdown()
{
	Game::Get()->GetCollisionSystem()->Clear();
	Game::Get()->GetCollisionSystem()->Shutdown();
	Game::Get()->GetScene()->Clear();
	Game::Get()->GetEventSystem()->Clear();

	return true;
}

/*virtual*/ bool Level::Tick(TickPass tickPass, double deltaTime)
{
	return true;
}

/*virtual*/ uint32_t Level::ShutdownOrder() const
{
	return std::numeric_limits<uint32_t>::max();
}