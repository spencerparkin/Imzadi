#include "Level.h"
#include "Game.h"
#include "Scene.h"
#include "Hero.h"
#include "AssetCache.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Assets/CollisionShapeSet.h"
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
	// TODO: We might load up a JSON file here for the level describing where all
	//       the enemies and trinkets go, as well as where the player start position is.

	std::string levelModelFile = std::format("Models/Level{}/Level{}.render_mesh", this->levelNumber, this->levelNumber);
	Game::Get()->LoadAndPlaceRenderMesh(levelModelFile, Vector3(), Quaternion());

	Hero* hero = Game::Get()->SpawnEntity<Hero>();
	hero->SetRestartLocation(Vector3(0.0, 0.0, 0.0));		// TODO: Maybe get this from the level description file?

	std::string staticCollisionFile = std::format("Models/Level{}/Level{}.collision", this->levelNumber, this->levelNumber);
	Reference<Asset> collisionAsset;
	if (!Game::Get()->GetAssetCache()->GrabAsset(staticCollisionFile, collisionAsset))
		return false;

	auto collisionShapeSet = dynamic_cast<CollisionShapeSet*>(collisionAsset.Get());
	if (!collisionShapeSet)
		return false;

	AxisAlignedBoundingBox boundingBox;
	if (!collisionShapeSet->GetBoundingBox(boundingBox))
		return false;

	boundingBox.Scale(1.5);
	if (!Game::Get()->GetCollisionSystem()->Initialize(boundingBox))
		return false;

	for (Shape* shape : collisionShapeSet->GetCollisionShapeArray())
		Game::Get()->GetCollisionSystem()->AddShape(shape, 0 /*COLL_SYS_ADD_FLAG_ALLOW_SPLIT*/);	// TODO: Figure out why splitting fails.

	collisionShapeSet->Clear(false);
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
	return true;
}