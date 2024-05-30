#include "Level.h"
#include "Game.h"
#include "Scene.h"
#include "Hero.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
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
	if (!Game::Get()->LoadStaticCollision(staticCollisionFile))
		return false;

	return true;
}

/*virtual*/ bool Level::Shutdown(bool gameShuttingDown)
{
	// TODO: Clean-up here.

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