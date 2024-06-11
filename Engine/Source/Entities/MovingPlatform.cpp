#include "MovingPlatform.h"
#include "Assets/MovingPlatformData.h"
#include "Assets/CollisionShapeSet.h"
#include "Command.h"
#include "Game.h"

using namespace Collision;

MovingPlatform::MovingPlatform()
{
	this->targetDeltaIndex = 0;
	this->bounceDelta = 1;
}

/*virtual*/ MovingPlatform::~MovingPlatform()
{
}

/*virtual*/ bool MovingPlatform::Setup()
{
	if (this->movingPlatformFile.size() == 0)
		return false;

	Reference<Asset> asset;
	if (!Game::Get()->GetAssetCache()->LoadAsset(this->movingPlatformFile, asset))
		return false;

	this->data.SafeSet(asset.Get());
	if (!this->data)
		return false;

	if (this->data->GetSplineDeltaArray().size() < 2)
		return false;

	std::string meshFile = this->data->GetMeshFile();
	Reference<RenderObject> renderObject = Game::Get()->LoadAndPlaceRenderMesh(meshFile, Vector3(), Quaternion());
	if (!renderObject)
		return false;

	this->renderMesh.SafeSet(renderObject.Get());
	if (!this->renderMesh)
		return false;

	std::string collisionFile = this->data->GetCollisionFile();
	if (!Game::Get()->GetAssetCache()->LoadAsset(collisionFile, asset))
		return false;

	auto collisionShapeSet = dynamic_cast<CollisionShapeSet*>(asset.Get());
	if (!collisionShapeSet)
		return false;

	this->collisionShapeArray.clear();
	for (Shape* shape : collisionShapeSet->GetCollisionShapeArray())
	{
		ShapeID shapeID = Game::Get()->GetCollisionSystem()->AddShape(shape, 0);
		this->collisionShapeArray.push_back(shapeID);
	}

	collisionShapeSet->Clear(false);

	this->targetDeltaIndex = 0;
	this->bounceDelta = 1;

	return true;
}

/*virtual*/ bool MovingPlatform::Shutdown(bool gameShuttingDown)
{
	this->collisionShapeArray.clear();

	return true;
}

/*virtual*/ bool MovingPlatform::Tick(TickPass tickPass, double deltaTime)
{
	if (tickPass != TickPass::PRE_TICK)
		return true;

	const Vector3& targetDelta = this->data->GetSplineDeltaArray()[this->targetDeltaIndex];

	Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
	bool arrived = false;
	double epsilon = 1e-3;

	switch (this->data->GetSplineType())
	{
		case MovingPlatformData::SplineType::LERP:
		{
			Vector3 moveDirection = targetDelta - objectToWorld.translation;
			double distance = 0.0;
			if (!moveDirection.Normalize(&distance) || distance < epsilon)
				arrived = true;
			else
			{
				Vector3 moveVelocity = moveDirection * this->data->GetMoveSpeed();
				Vector3 moveDelta = moveVelocity * deltaTime;
				if (moveDelta.Length() > distance)
					moveDelta = moveDirection * distance;
				objectToWorld.translation += moveDelta;
			}

			break;
		}
		case MovingPlatformData::SplineType::SMOOTH:
		{
			// TODO: Write this.
			break;
		}
	}

	this->renderMesh->SetObjectToWorldTransform(objectToWorld);

	for (ShapeID shapeID : this->collisionShapeArray)
	{
		auto command = ObjectToWorldCommand::Create();
		command->SetShapeID(shapeID);
		command->objectToWorld = objectToWorld;
		Game::Get()->GetCollisionSystem()->IssueCommand(command);
	}

	if (arrived)
	{
		switch (this->data->GetSplineMode())
		{
			case MovingPlatformData::SplineMode::BOUNCE:
			{
				this->targetDeltaIndex += this->bounceDelta;

				if (this->targetDeltaIndex < 0)
				{
					this->targetDeltaIndex = 1;
					this->bounceDelta = 1;
				}
				else if (this->targetDeltaIndex >= this->data->GetSplineDeltaArray().size())
				{
					this->targetDeltaIndex = signed(this->data->GetSplineDeltaArray().size()) - 1;
					this->bounceDelta = -1;
				}

				break;
			}
			case MovingPlatformData::SplineMode::CYCLE:
			{
				this->targetDeltaIndex = (this->targetDeltaIndex + 1) % this->data->GetSplineDeltaArray().size();
				break;
			}
		}
	}

	return true;
}