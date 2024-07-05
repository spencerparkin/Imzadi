#include "MovingPlatform.h"
#include "Math/Angle.h"
#include "Assets/MovingPlatformData.h"
#include "Assets/CollisionShapeSet.h"
#include "Assets/RenderMesh.h"
#include "Collision/Command.h"
#include "Game.h"

using namespace Imzadi;

MovingPlatform::MovingPlatform()
{
	this->targetDeltaIndex = 0;
	this->bounceDelta = 1;
	this->state = State::UNKNOWN;
	this->remainingLingerTimeSeconds = 0.0;
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
	Reference<RenderObject> renderObject = Game::Get()->LoadAndPlaceRenderMesh(meshFile);
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

	const MovingPlatformData::DeltaInfo& targetDelta = this->data->GetSplineDeltaArray()[this->targetDeltaIndex];
	this->remainingLingerTimeSeconds = targetDelta.lingerTimeSeconds;
	this->state = State::LINGERING;
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

	if (this->state == State::LINGERING)
	{
		this->remainingLingerTimeSeconds -= deltaTime;

		// Have we lingered long enough?
		if (this->remainingLingerTimeSeconds <= 0.0)
		{
			// Yes.  Determine the next target delta and move into the moving state.
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

			this->state = State::MOVING;
		}
	}
	
	if (this->state == State::MOVING)
	{
		const MovingPlatformData::DeltaInfo& targetDelta = this->data->GetSplineDeltaArray()[this->targetDeltaIndex];

		const Transform& currentObjectToWorld = this->renderMesh->GetObjectToWorldTransform();
		const Transform& originalObjectToWorld = this->renderMesh->GetRenderMesh()->GetObjectToWorldTransform();

		Transform targetObjectToWorld;
		targetObjectToWorld.translation = originalObjectToWorld.translation + targetDelta.transform.translation;
		targetObjectToWorld.matrix = targetDelta.transform.matrix * originalObjectToWorld.matrix;

		double translationalStep = this->data->GetMoveSpeed() * deltaTime;
		double rotationalStep = this->data->GetRotationSpeed() * deltaTime;

		// Have we reached our destination?
		Transform newObjectToWorld;
		if (!newObjectToWorld.MoveTo(currentObjectToWorld, targetObjectToWorld, translationalStep, rotationalStep))
		{
			// Yes.  Go into the lingering state.
			this->remainingLingerTimeSeconds = targetDelta.lingerTimeSeconds;
			this->state = State::LINGERING;
		}
		else
		{
			// No.  Move the platform for this frame.
			this->renderMesh->SetObjectToWorldTransform(newObjectToWorld);
		}

		for (ShapeID shapeID : this->collisionShapeArray)
		{
			auto command = ObjectToWorldCommand::Create();
			command->SetShapeID(shapeID);
			command->objectToWorld = this->renderMesh->GetObjectToWorldTransform();
			Game::Get()->GetCollisionSystem()->IssueCommand(command);
		}
	}

	return true;
}