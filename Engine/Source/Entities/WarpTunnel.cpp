#include "WarpTunnel.h"
#include "Assets/WarpTunnelData.h"
#include "Assets/CollisionShapeSet.h"
#include "Assets/RenderMesh.h"
#include "Collision/Command.h"
#include "Log.h"

using namespace Imzadi;

WarpTunnel::WarpTunnel()
{
	this->mainCharacterHandle = 0;
	this->currentlyBoundPortNumber = -1;
	this->coolDownCount = 0;
	this->eventListenerHandle = 0;
}

/*virtual*/ WarpTunnel::~WarpTunnel()
{
}

/*virtual*/ bool WarpTunnel::Setup()
{
	if (this->warpTunnelFile.length() == 0)
	{
		IMZADI_LOG_ERROR("No warp tunnel file configured.");
		return false;
	}

	if (this->mainCharacterHandle == 0)
	{
		IMZADI_LOG_ERROR("No main character handle given.");
		return false;
	}

	Reference<ReferenceCounted> entityRef;
	if (!HandleManager::Get()->GetObjectFromHandle(this->mainCharacterHandle, entityRef))
	{
		IMZADI_LOG_ERROR("Failed to dereference main character handle.");
		return false;
	}

	this->targetEntity.SafeSet(entityRef.Get());
	if (!this->targetEntity)
	{
		IMZADI_LOG_ERROR("Main character handle didn't dereference as an entity.");
		return false;
	}

	Reference<Asset> asset;
	if (!Game::Get()->GetAssetCache()->LoadAsset(this->warpTunnelFile, asset))
	{
		IMZADI_LOG_ERROR("Failed to load warp tunnel data file: %s", this->warpTunnelFile.c_str());
		return false;
	}

	this->data.SafeSet(asset.Get());
	if (!this->data)
	{
		IMZADI_LOG_ERROR("Data loaded for warp tunnel was not warp tunnel data.");
		return false;
	}

	std::string meshFile = this->data->GetMeshFile();
	Reference<RenderObject> renderObject = Game::Get()->LoadAndPlaceRenderMesh(meshFile);
	if (!renderObject)
	{
		IMZADI_LOG_ERROR("Could not load and place render mesh: %s", meshFile.c_str());
		return false;
	}

	this->renderMesh.SafeSet(renderObject.Get());
	if (!this->renderMesh)
	{
		IMZADI_LOG_ERROR("Whatever loaded for the render mesh wasn't a render mesh instance.");
		return false;
	}

	std::string collisionFile = this->data->GetCollisionFile();
	if (!Game::Get()->GetAssetCache()->LoadAsset(collisionFile, asset))
	{
		IMZADI_LOG_ERROR("Failed to load collision file: %s", collisionFile.c_str());
		return false;
	}

	auto collisionShapeSet = dynamic_cast<CollisionShapeSet*>(asset.Get());
	if (!collisionShapeSet)
	{
		IMZADI_LOG_ERROR("Whatever loaded as collision wasn't a collision shape set.");
		return false;
	}

	this->collisionShapeSet.clear();
	for (Collision::Shape* shape : collisionShapeSet->GetCollisionShapeArray())
	{
		Collision::ShapeID shapeID = Game::Get()->GetCollisionSystem()->AddShape(shape, 0);
		if (this->collisionShapeSet.find(shapeID) != this->collisionShapeSet.end())
		{
			IMZADI_LOG_ERROR("Duplicate shape ID encountered.");
			return false;
		}

		this->collisionShapeSet.insert(shapeID);
	}

	collisionShapeSet->Clear(false);

	if (!this->BindPort(0))
	{
		IMZADI_LOG_ERROR("Initial port bind failed.");
		return false;
	}

	this->eventListenerHandle = Game::Get()->GetEventSystem()->RegisterEventListener("Biped", new LambdaEventListener([=](const Event* event) {
		this->HandleBipedResetEvent(dynamic_cast<const BipedResetEvent*>(event));
	}));

	return true;
}

/*virtual*/ bool WarpTunnel::Shutdown()
{
	if (this->eventListenerHandle != 0)
	{
		// We don't want the event listener to hang on to a lambda having a capture value that is a stale pointer.
		Game::Get()->GetEventSystem()->UnregisterEventListener(this->eventListenerHandle);
		this->eventListenerHandle = 0;
	}

	Game::Get()->GetScene()->RemoveRenderObject(this->renderMesh->GetName());

	for (Collision::ShapeID shapeID : this->collisionShapeSet)
		Game::Get()->GetCollisionSystem()->RemoveShape(shapeID);

	this->collisionShapeSet.clear();
	this->targetEntity.Reset();
	this->currentlyBoundPortNumber = -1;

	return true;
}

void WarpTunnel::HandleBipedResetEvent(const BipedResetEvent* event)
{
	if (!event)
		return;

	if (event->handle != this->targetEntity->GetHandle())
		return;

	this->BindPort(0);
}

/*virtual*/ bool WarpTunnel::Tick(TickPass tickPass, double deltaTime)
{
	if (tickPass != TickPass::MOVE_UNCONSTRAINTED)
		return true;

	if (this->coolDownCount > 0)
	{
		this->coolDownCount--;
		return true;
	}

	Collision::ShapeID groundShapeID = this->targetEntity->GetGroundContactShape();
	if (this->collisionShapeSet.find(groundShapeID) == this->collisionShapeSet.end())
		return true;

	Transform objectToWorld;
	if (!this->targetEntity->GetTransform(objectToWorld))
		return false;

	double smallestSquareDistance = std::numeric_limits<double>::max();
	int portNumber = -1;

	const auto& portBindArray = this->data->GetPortBindArray();
	for (int i = 0; i < (int)portBindArray.size(); i++)
	{
		const WarpTunnelData::PortBind& portBind = portBindArray[i];

		Transform domesticPortToObject;
		this->renderMesh->GetRenderMesh()->GetPort(portBind.domesticPort, domesticPortToObject);

		Transform domesticObjectToWorld = this->renderMesh->GetObjectToWorldTransform();
		Transform domesticPortToWorld = domesticObjectToWorld * domesticPortToObject;

		Vector3 delta = domesticPortToWorld.translation - objectToWorld.translation;
		double squareDistance = delta.SquareLength();
		if (squareDistance < smallestSquareDistance)
		{
			portNumber = i;
			smallestSquareDistance = squareDistance;
		}
	}

	IMZADI_ASSERT(portNumber != -1);

	if (portNumber != this->currentlyBoundPortNumber)
	{
		IMZADI_LOG_INFO("Warp tunnel binding to port %d.", portNumber);

		Game::Get()->GetEventSystem()->SendEventNow("WarpTunnel", new WarpTunnelEvent());

		if (this->BindPort(portNumber))
		{
			// This is to prevent us from flipping back and forth rappedly due
			// possibly to round-off error.  This seems hacky, but it's not that
			// bad, I think.  Something bad happens when we rappidly flip back
			// and forth and it causes the character to get abandoned in outer
			// space and then die.
			this->coolDownCount = 20;
		}
		else
		{
			IMZADI_LOG_ERROR("Warp tunnel bind failed!");
			return false;
		}
	}

	return true;
}

bool WarpTunnel::BindPort(int portNumber)
{
	if (this->currentlyBoundPortNumber == portNumber)
		return true;

	const auto& portBindArray = this->data->GetPortBindArray();
	if (portNumber < 0 || portNumber >= portBindArray.size())
		return false;

	const WarpTunnelData::PortBind& portBind = portBindArray[portNumber];
	Transform domesticPortToObject;
	if (!this->renderMesh->GetRenderMesh()->GetPort(portBind.domesticPort, domesticPortToObject))
		return false;

	Reference<RenderObject> foreignRenderObject;
	if (!Game::Get()->GetScene()->FindRenderObject(portBind.foreignMesh, foreignRenderObject))
		return false;

	auto foreignRenderMesh = dynamic_cast<RenderMeshInstance*>(foreignRenderObject.Get());
	Transform foreignPortToObject;
	if (!foreignRenderMesh->GetRenderMesh()->GetPort(portBind.foreignPort, foreignPortToObject))
		return false;

	Transform rotation;
	rotation.translation.SetComponents(0.0, 0.0, 0.0);
	rotation.matrix.SetFromAxisAngle(Vector3(0.0, 1.0, 0.0), M_PI);

	Transform domesticObjectToPort;
	if (!domesticObjectToPort.Invert(domesticPortToObject))
		return false;

	Transform foreignObjectToWorld = foreignRenderMesh->GetObjectToWorldTransform();
	Transform foreignPortToWorld = foreignObjectToWorld * foreignPortToObject;
	Transform domesticObjectToWorld = foreignPortToWorld * rotation * domesticObjectToPort;

	this->renderMesh->SetObjectToWorldTransform(domesticObjectToWorld);

	for (Collision::ShapeID shapeID : this->collisionShapeSet)
	{
		auto command = new Collision::ObjectToWorldCommand();
		command->SetShapeID(shapeID);
		command->objectToWorld = this->renderMesh->GetObjectToWorldTransform();
		Game::Get()->GetCollisionSystem()->IssueCommand(command);
	}

	this->currentlyBoundPortNumber = portNumber;

	return true;
}