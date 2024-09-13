#include "Riker.h"
#include "Assets/RenderMesh.h"

Riker::Riker()
{
	this->SetName("Riker");
	this->waypointTargetIndex = -1;
	this->runSpeed = 2.0;
}

/*virtual*/ Riker::~Riker()
{
}

/*virtual*/ bool Riker::Setup()
{
	std::string modelFile = "Models/Riker/Riker.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	Imzadi::Reference<Imzadi::RenderObject> renderObject;
	if (!Imzadi::Game::Get()->GetScene()->FindRenderObject("Tube002", renderObject))
		return false;

	auto tubeMesh = dynamic_cast<Imzadi::RenderMeshInstance*>(renderObject.Get());
	if (!tubeMesh)
		return false;

	Imzadi::Transform objectToWorld = tubeMesh->GetObjectToWorldTransform();
	this->waypointLoopArray.clear();
	int i = 0;
	while (true)
	{
		Imzadi::Transform portToObject;
		if (!tubeMesh->GetRenderMesh()->GetPort(std::format("Port{}", i++), portToObject))
			break;

		Imzadi::Transform portToWorld = objectToWorld * portToObject;
		this->waypointLoopArray.push_back(portToWorld.translation);
	}

	//tubeMesh->SetDrawPorts(true);

	this->waypointTargetIndex = this->restartTransformObjectToWorld.translation.NearestPoint(this->waypointLoopArray);
	if (this->waypointTargetIndex == -1)
		return false;

	return true;
}

/*virtual*/ bool Riker::Shutdown()
{
	Character::Shutdown();

	return true;
}

/*virtual*/ bool Riker::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	if (tickPass == Imzadi::TickPass::PARALLEL_WORK)
	{
		const Imzadi::Transform& objectToWorld = this->renderMesh->GetObjectToWorldTransform();
		static double squareRadius = 4.0;
		int i = objectToWorld.translation.NearestPoint(this->waypointLoopArray, squareRadius);
		if (i == this->waypointTargetIndex)
			this->waypointTargetIndex = (i + 1) % this->waypointLoopArray.size();
	}

	return true;
}

/*virtual*/ void Riker::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	if (this->inContactWithGround)
	{
		const Imzadi::Vector3& waypointTarget = this->waypointLoopArray[this->waypointTargetIndex];
		const Imzadi::Transform& objectToWorld = renderMesh->GetObjectToWorldTransform();

		this->velocity = waypointTarget - objectToWorld.translation;
		this->velocity = this->velocity.RejectedFrom(this->groundSurfaceNormal).Normalized() * this->runSpeed;
	}

	Character::IntegrateVelocity(acceleration, deltaTime);
}

/*virtual*/ std::string Riker::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "RikerIdle";
	case Imzadi::Biped::AnimType::RUN:
		return "RikerRun";
	}

	return "";
}