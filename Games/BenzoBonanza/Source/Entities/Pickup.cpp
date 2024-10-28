#include "Pickup.h"
#include "GameApp.h"
#include "Assets/GameProgress.h"
#include "Collision/Shapes/Sphere.h"
#include "Collision/Command.h"
#include "Assets/RenderMesh.h"
#include "Log.h"
#include "Audio/System.h"

//----------------------------------- Pickup -----------------------------------

Pickup::Pickup()
{
	this->pickupShapeID = 0;
}

/*virtual*/ Pickup::~Pickup()
{
}

/*virtual*/ bool Pickup::Setup()
{
	if (this->renderMeshFile.length() == 0)
	{
		IMZADI_LOG_ERROR("Pickups need to specify their render mesh in their constructor.");
		return false;
	}

	auto* game = Imzadi::Game::Get();

	Imzadi::Reference<Imzadi::Asset> asset;
	if (!game->GetAssetCache()->LoadAsset(this->renderMeshFile, asset))
	{
		IMZADI_LOG_ERROR("Failed to load pickup: %s", this->renderMeshFile.c_str());
		return false;
	}

	Imzadi::Reference<Imzadi::RenderMeshAsset> renderMeshAsset;
	renderMeshAsset.SafeSet(asset.Get());
	if (!renderMeshAsset)
	{
		IMZADI_LOG_ERROR("Didn't load render mesh for pickup: %s.", this->renderMeshFile.c_str());
		return false;
	}

	Imzadi::Reference<Imzadi::RenderObject> renderObject;
	if (!renderMeshAsset->MakeRenderInstance(renderObject))
	{
		IMZADI_LOG_ERROR("Failed to instance render mesh for pickup: %s", this->renderMeshFile.c_str());
		return false;
	}

	this->renderMesh.SafeSet(renderObject.Get());
	if (!this->renderMesh)
	{
		IMZADI_LOG_ERROR("Expected to instance a render mesh for pickup: %s", this->renderMeshFile.c_str());
		return false;
	}

	if (!game->GetScene()->AddRenderObject(renderObject))
	{
		IMZADI_LOG_ERROR("Failed to add render mesh to scene for pickup: %s", this->renderMeshFile.c_str());
		return false;
	}

	this->renderMesh->SetObjectToWorldTransform(this->initialTransform);

	auto sphere = new Imzadi::Collision::SphereShape();
	sphere->SetRadius(2.0);		// All pickups are assumed to be the same general size, so we can hard-code this, I think.
	sphere->SetUserFlags(SHAPE_FLAG_PICKUP);
	this->pickupShapeID = game->GetCollisionSystem()->AddShape(sphere, 0);
	
	auto command = new Imzadi::Collision::ObjectToWorldCommand();
	command->SetShapeID(this->pickupShapeID);
	command->objectToWorld = this->initialTransform;
	game->GetCollisionSystem()->IssueCommand(command);

	return true;
}

/*virtual*/ bool Pickup::Shutdown()
{
	auto* game = Imzadi::Game::Get();

	if (this->renderMesh.Get())
	{
		game->GetScene()->RemoveRenderObject(this->renderMesh->GetName());
		this->renderMesh.Reset();
	}

	if (this->pickupShapeID != 0)
	{
		game->GetCollisionSystem()->RemoveShape(this->pickupShapeID);
		this->pickupShapeID = 0;
	}

	return true;
}

/*virtual*/ bool Pickup::SetTransform(const Imzadi::Transform& transform)
{
	this->initialTransform = transform;
	return true;
}

/*virtual*/ void Pickup::Configure(const std::unordered_map<std::string, std::string>& configMap)
{
}

/*virtual*/ bool Pickup::OwnsCollisionShape(Imzadi::Collision::ShapeID shapeID) const
{
	return this->pickupShapeID == shapeID;
}

/*virtual*/ bool Pickup::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Entity::Tick(tickPass, deltaTime))
		return false;

	if (tickPass == Imzadi::TickPass::MOVE_UNCONSTRAINTED)
	{
		// Animate the pick-up by just slowly rotating it on the Y-axis.
		static double rotationRate = 0.5;
		Imzadi::Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
		Imzadi::Matrix3x3 matrix;
		matrix.SetFromAxisAngle(Imzadi::Vector3(0.0, 1.0, 0.0), rotationRate * deltaTime);
		objectToWorld.matrix = (matrix * objectToWorld.matrix).Orthonormalized(IMZADI_AXIS_FLAG_X);
		this->renderMesh->SetObjectToWorldTransform(objectToWorld);
	}

	return true;
}

/*virtual*/ void Pickup::Collect()
{
	this->DoomEntity();
}

/*virtual*/ std::string Pickup::GetVerb() const
{
	return "pickup";
}

/*virtual*/ bool Pickup::CanBePickedUp() const
{
	return true;
}

//----------------------------------- ExtraLifePickup -----------------------------------

ExtraLifePickup::ExtraLifePickup()
{
	this->renderMeshFile = "Models/Heart/Heart.render_mesh";
}

/*virtual*/ ExtraLifePickup::~ExtraLifePickup()
{
}

/*virtual*/ void ExtraLifePickup::Collect()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	GameProgress* gameProgress = game->GetGameProgress();
	gameProgress->SetNumLives(gameProgress->GetNumLives() + 1);
	
	game->GetAudioSystem()->PlaySound("Yay");

	Pickup::Collect();
}

/*virtual*/ std::string ExtraLifePickup::GetLabel() const
{
	return "extra life";
}

//----------------------------------- SpeedBoostPickup -----------------------------------

SpeedBoostPickup::SpeedBoostPickup()
{
	this->renderMeshFile = "Models/SpeedBoost/EnergyDrink.render_mesh";
}

/*virtual*/ SpeedBoostPickup::~SpeedBoostPickup()
{
}

/*virtual*/ void SpeedBoostPickup::Collect()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	GameProgress* progress = game->GetGameProgress();
	uint32_t count = progress->GetPossessedItemCount("booster");
	progress->SetPossessedItemCount("booster", count + 1);

	Pickup::Collect();
}

/*virtual*/ std::string SpeedBoostPickup::GetLabel() const
{
	return "speed-booster";
}

//----------------------------------- SongPickup -----------------------------------

SongPickup::SongPickup()
{
	this->renderMeshFile = "Models/MusicalNote/MusicalNote.render_mesh";
}

/*virtual*/ SongPickup::~SongPickup()
{
}

/*virtual*/ void SongPickup::Collect()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	auto audioSystem = game->GetAudioSystem();

	if (!audioSystem->IsMidiSongPlaying())
	{
		if (!audioSystem->PlaySound(this->song))
		{
			IMZADI_LOG_WARNING("Could not play: %s", this->song.c_str());
		}
	}
}

/*virtual*/ std::string SongPickup::GetLabel() const
{
	return "music";
}

/*virtual*/ std::string SongPickup::GetVerb() const
{
	return "play";
}

/*virtual*/ void SongPickup::Configure(const std::unordered_map<std::string, std::string>& configMap)
{
	auto iter = configMap.find("song");
	if (iter != configMap.end())
		this->song = iter->second;
}

/*virtual*/ bool SongPickup::CanBePickedUp() const
{
	auto game = (GameApp*)Imzadi::Game::Get();
	auto audioSystem = game->GetAudioSystem();

	return !audioSystem->IsMidiSongPlaying();
}

//----------------------------------- KeyPickup -----------------------------------

KeyPickup::KeyPickup()
{
	this->renderMeshFile = "Models/Key/Key.render_mesh";
}

/*virtual*/ KeyPickup::~KeyPickup()
{
}

/*virtual*/ void KeyPickup::Collect()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	uint32_t keyCount = game->GetGameProgress()->GetPossessedItemCount("key");
	game->GetGameProgress()->SetPossessedItemCount("key", keyCount + 1);

	game->GetAudioSystem()->PlaySound("Yay");

	Pickup::Collect();
}

/*virtual*/ std::string KeyPickup::GetLabel() const
{
	return "key";
}

//----------------------------------- BenzoPickup -----------------------------------

BenzoPickup::BenzoPickup()
{
}

/*virtual*/ BenzoPickup::~BenzoPickup()
{
}

/*virtual*/ bool BenzoPickup::Setup()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	GameProgress* progress = game->GetGameProgress();
	if (progress->WasBenzoCollectedAt(this->initialTransform.translation))
		return false;

	return Pickup::Setup();
}

/*virtual*/ void BenzoPickup::Collect()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	GameProgress* progress = game->GetGameProgress();
	progress->SetBenzoCollectedAt(this->initialTransform.translation);
	uint32_t count = progress->GetPossessedItemCount(this->benzoType);
	progress->SetPossessedItemCount(this->benzoType, count + 1);

	game->GetAudioSystem()->PlaySound("Yay");

	Pickup::Collect();
}

/*virtual*/ std::string BenzoPickup::GetLabel() const
{
	return this->benzoType;
}

/*virtual*/ void BenzoPickup::Configure(const std::unordered_map<std::string, std::string>& configMap)
{
	std::unordered_map<std::string, std::string>::const_iterator iter = configMap.find("type");
	if (iter == configMap.end())
		return;

	this->benzoType = iter->second;
	IMZADI_ASSERT(IsBenzoName(benzoType));
	this->renderMeshFile = std::format("Models/Benzos/{}.render_mesh", this->benzoType);
}

/*static*/ bool BenzoPickup::IsBenzoName(const std::string& name)
{
	return(
		name == "Ativan" ||
		name == "Halcion" ||
		name == "Klonopin" ||
		name == "Librium" ||
		name == "Restoril" ||
		name == "Valium" ||
		name == "Xanax"
	);
}