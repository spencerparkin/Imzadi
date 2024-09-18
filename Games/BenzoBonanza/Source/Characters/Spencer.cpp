#include "Spencer.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "GameApp.h"
#include "Math/Random.h"

Spencer::Spencer()
{
	this->SetName("Spencer");
	this->disposition = Disposition::COLLECT_BENZOS_FROM_PLAYER;
	this->runSpeed = 10.0;
	this->canRestart = false;
}

/*virtual*/ Spencer::~Spencer()
{
}

/*virtual*/ void Spencer::ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Imzadi::Vector3(0.0, 1.0, 0.0));
	capsule->SetVertex(1, Imzadi::Vector3(0.0, 5.0, 0.0));
	capsule->SetRadius(1.0);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY | SHAPE_FLAG_TALKER);
}

/*virtual*/ bool Spencer::Setup()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	if (game->GetGameProgress()->WasMileStoneReached("spencer_suicide"))
		return false;

	std::string modelFile = "Models/Spencer/Spencer.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	Imzadi::Random random;
	Imzadi::Vector3 upVector(0.0, 1.0, 0.0);
	do
	{
		this->runDirection.SetAsRandomDirection(random);
		this->runDirection = this->runDirection.RejectedFrom(upVector);
	} while (!this->runDirection.Normalize());

	game->GetEventSystem()->RegisterEventListener(
		"Spencer",
		Imzadi::EventListenerType::TRANSITORY,
		new Imzadi::LambdaEventListener([=](const Imzadi::Event* event)
			{
				this->HandleEvent(event);
			}));

	return true;
}

/*virtual*/ bool Spencer::Shutdown()
{
	Character::Shutdown();
	return true;
}

void Spencer::HandleEvent(const Imzadi::Event* event)
{
	if (event && event->GetName() == "suicide")
		this->disposition = Disposition::COMMIT_SUICIDE;
}

/*virtual*/ void Spencer::AdjustFacingDirection(double deltaTime)
{
	if (this->disposition == Disposition::COLLECT_BENZOS_FROM_PLAYER)
	{
		Imzadi::Reference<Imzadi::Entity> entity;
		if (Imzadi::Game::Get()->FindEntityByName("Alice", entity))
		{
			Imzadi::Transform aliceTransform;
			entity->GetTransform(aliceTransform);

			Imzadi::Transform spencerTransform;
			this->GetTransform(spencerTransform);

			Imzadi::Vector3 direction = (aliceTransform.translation - spencerTransform.translation).Normalized();
			Imzadi::Vector3 xAxis, yAxis, zAxis;
			yAxis.SetComponents(0.0, 1.0, 0.0);
			zAxis = (-direction).RejectedFrom(yAxis).Normalized();
			xAxis = yAxis.Cross(zAxis);
			Imzadi::Matrix3x3 matrix;
			matrix.SetColumnVectors(xAxis, yAxis, zAxis);
			if (matrix.IsValid())
				this->objectToPlatform.matrix = matrix;		// This assumes the platform-to-world matrix is identity.
		}
	}
	else if (this->disposition == Disposition::COMMIT_SUICIDE)
		Character::AdjustFacingDirection(deltaTime);
}

/*virtual*/ bool Spencer::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	return true;
}

/*virtual*/ void Spencer::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	if (this->disposition == Disposition::COLLECT_BENZOS_FROM_PLAYER)
	{
		this->velocity.x = 0.0;
		this->velocity.z = 0.0;
	}
	else
	{
		if (this->inContactWithGround)
			this->velocity = this->runDirection * this->runSpeed;
	}

	Character::IntegrateVelocity(acceleration, deltaTime);
}

/*virtual*/ std::string Spencer::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "SpencerIdle";
	case Imzadi::Biped::AnimType::RUN:
		return "SpencerRun";
	}

	return "";
}

/*virtual*/ bool Spencer::OnBipedDied()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	game->GetGameProgress()->SetMileStoneReached("spencer_suicide");

	return Character::OnBipedDied();
}