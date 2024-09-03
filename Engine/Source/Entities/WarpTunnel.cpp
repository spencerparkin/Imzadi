#include "WarpTunnel.h"
#include "Assets/WarpTunnelData.h"
#include "Assets/CollisionShapeSet.h"
#include "Assets/RenderMesh.h"
#include "Collision/Command.h"

using namespace Imzadi;

WarpTunnel::WarpTunnel()
{
}

/*virtual*/ WarpTunnel::~WarpTunnel()
{
}

/*virtual*/ bool WarpTunnel::Setup()
{
	if (this->warpTunnelFile.length() == 0)
		return false;

	Reference<Asset> asset;
	if (!Game::Get()->GetAssetCache()->LoadAsset(this->warpTunnelFile, asset))
		return false;

	this->data.SafeSet(asset.Get());
	if (!this->data)
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
	for (Collision::Shape* shape : collisionShapeSet->GetCollisionShapeArray())
	{
		Collision::ShapeID shapeID = Game::Get()->GetCollisionSystem()->AddShape(shape, 0);
		this->collisionShapeArray.push_back(shapeID);
	}

	collisionShapeSet->Clear(false);

	if (!this->BindPort(0))
		return false;

	return true;
}

/*virtual*/ bool WarpTunnel::Shutdown()
{
	Game::Get()->GetScene()->RemoveRenderObject(this->renderMesh->GetName());

	for (Collision::ShapeID shapeID : this->collisionShapeArray)
		Game::Get()->GetCollisionSystem()->RemoveShape(shapeID);

	this->collisionShapeArray.clear();

	return true;
}

/*virtual*/ bool WarpTunnel::Tick(TickPass tickPass, double deltaTime)
{
	return true;
}

bool WarpTunnel::BindPort(int portNumber)
{
	if (portNumber < 0 || portNumber >= this->data->GetPortBindArray().size())
		return false;

	const WarpTunnelData::PortBind& portBind = this->data->GetPortBindArray()[portNumber];
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

	for (Collision::ShapeID shapeID : this->collisionShapeArray)
	{
		auto command = new Collision::ObjectToWorldCommand();
		command->SetShapeID(shapeID);
		command->objectToWorld = this->renderMesh->GetObjectToWorldTransform();
		Game::Get()->GetCollisionSystem()->IssueCommand(command);
	}

	return true;
}