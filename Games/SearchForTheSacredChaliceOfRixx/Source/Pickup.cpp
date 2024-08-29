#include "Pickup.h"
#include "GameApp.h"
#include "Assets/GameProgress.h"
#include "Collision/Shapes/Sphere.h"
#include "Collision/Command.h"
#include "Assets/RenderMesh.h"
#include "Log.h"

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

	game->GetScene()->RemoveRenderObject(this->renderMesh->GetName());
	game->GetCollisionSystem()->RemoveShape(this->pickupShapeID);
	this->pickupShapeID = 0;

	return true;
}

/*virtual*/ bool Pickup::SetTransform(const Imzadi::Transform& transform)
{
	this->initialTransform = transform;
	return true;
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

	Pickup::Collect();
}

/*virtual*/ std::string ExtraLifePickup::GetLabel() const
{
	return "extra life";
}

//----------------------------------- SpeedBoostPickup -----------------------------------

SpeedBoostPickup::SpeedBoostPickup()
{
	this->renderMeshFile = "Models/SpeedBoost/SpeedBoost.render_mesh";
}

/*virtual*/ SpeedBoostPickup::~SpeedBoostPickup()
{
}

/*virtual*/ void SpeedBoostPickup::Collect()
{
	// TODO: Write this.

	Pickup::Collect();
}

/*virtual*/ std::string SpeedBoostPickup::GetLabel() const
{
	return "speed-booster";
}