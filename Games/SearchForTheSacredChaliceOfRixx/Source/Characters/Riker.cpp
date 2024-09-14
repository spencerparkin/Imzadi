#include "Riker.h"
#include "Assets/RenderMesh.h"
#include "GameApp.h"

Riker::Riker()
{
	this->SetName("Riker");
	this->waypointTargetIndex = -1;
	this->runSpeed = 10.0;
	this->disposition = Disposition::RUN_AROUND_LIKE_AN_IDIOT;
}

/*virtual*/ Riker::~Riker()
{
}

/*virtual*/ void Riker::ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Imzadi::Vector3(0.0, 1.0, 0.0));
	capsule->SetVertex(1, Imzadi::Vector3(0.0, 5.0, 0.0));
	capsule->SetRadius(1.0);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY | SHAPE_FLAG_TALKER);
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

	Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener(
		"ConvoBoundary",
		Imzadi::EventListenerType::TRANSITORY,
		new Imzadi::LambdaEventListener([=](const Imzadi::Event* event)
			{
				this->HandleConversationBoundaryEvent(dynamic_cast<const ConvoBoundaryEvent*>(event));
			}));

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

	switch (tickPass)
	{
		case Imzadi::TickPass::PARALLEL_WORK:
		{
			if (this->disposition == Disposition::RUN_AROUND_LIKE_AN_IDIOT)
			{
				const Imzadi::Transform& objectToWorld = this->renderMesh->GetObjectToWorldTransform();
				static double squareRadius = 4.0;
				int i = objectToWorld.translation.NearestPoint(this->waypointLoopArray, squareRadius);
				if (i == this->waypointTargetIndex)
					this->waypointTargetIndex = (i + 1) % this->waypointLoopArray.size();
			}

			break;
		}
	}

	return true;
}

/*virtual*/ Imzadi::Vector3 Riker::GetPlatformSpaceFacingDirection() const
{
	if (this->disposition != Disposition::STOP_AND_TALK)
		return Character::GetPlatformSpaceFacingDirection();

	Imzadi::Transform objectToWorld = this->platformToWorld * this->objectToPlatform;
	
	Imzadi::Vector3 upDirection(0.0, 1.0, 0.0);
	Imzadi::Vector3 facingDirection = (this->talkTarget - objectToWorld.translation).RejectedFrom(upDirection).Normalized();
	
	Imzadi::Transform worldToPlatform;
	worldToPlatform.Invert(this->platformToWorld);

	return worldToPlatform.TransformVector(facingDirection);
}

/*virtual*/ void Riker::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	if (this->inContactWithGround)
	{
		switch (this->disposition)
		{
			case Disposition::RUN_AROUND_LIKE_AN_IDIOT:
			{
				const Imzadi::Vector3& waypointTarget = this->waypointLoopArray[this->waypointTargetIndex];
				const Imzadi::Transform& objectToWorld = renderMesh->GetObjectToWorldTransform();

				this->velocity = waypointTarget - objectToWorld.translation;
				this->velocity = this->velocity.RejectedFrom(this->groundSurfaceNormal).Normalized() * this->runSpeed;

				break;
			}
			case Disposition::STOP_AND_TALK:
			{
				this->velocity.SetComponents(0.0, 0.0, 0.0);
				break;
			}
		}
		
	}

	Character::IntegrateVelocity(acceleration, deltaTime);
}

void Riker::HandleConversationBoundaryEvent(const ConvoBoundaryEvent* event)
{
	if (event->IsParticipant(this->GetHandle()))
	{
		switch (event->type)
		{
			case ConvoBoundaryEvent::STARTED:
			{
				this->disposition = Disposition::STOP_AND_TALK;
				this->CalculateTalkTarget(event);
				break;
			}
			case ConvoBoundaryEvent::FINISHED:
			{
				this->disposition = Disposition::RUN_AROUND_LIKE_AN_IDIOT;
				break;
			}
		}
	}
}

void Riker::CalculateTalkTarget(const ConvoBoundaryEvent* event)
{
	this->talkTarget.SetComponents(0.0, 0.0, 0.0);
	uint32_t count = 0;

	for (uint32_t handle : event->participantHandleArray)
	{
		if (handle == this->GetHandle())
			continue;

		Imzadi::Reference<Imzadi::ReferenceCounted> ref;
		Imzadi::HandleManager::Get()->GetObjectFromHandle(handle, ref);
		Imzadi::Reference<Imzadi::Entity> entity;
		entity.SafeSet(ref.Get());
		if (!entity)
			continue;

		Imzadi::Transform objectToWorld;
		if (!entity->GetTransform(objectToWorld))
			continue;

		this->talkTarget += objectToWorld.translation;
		count++;
	}

	if (count > 0)
		this->talkTarget /= double(count);
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