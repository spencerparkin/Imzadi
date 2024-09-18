#include "Spencer.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "GameApp.h"

Spencer::Spencer()
{
	this->SetName("Spencer");
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
	std::string modelFile = "Models/Spencer/Spencer.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	return true;
}

/*virtual*/ bool Spencer::Shutdown()
{
	Character::Shutdown();
	return true;
}

/*virtual*/ void Spencer::AdjustFacingDirection(double deltaTime)
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

/*virtual*/ bool Spencer::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	return true;
}

/*virtual*/ void Spencer::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	Character::IntegrateVelocity(acceleration, deltaTime);

	this->velocity.x = 0.0;
	this->velocity.z = 0.0;
}

/*virtual*/ std::string Spencer::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "SpencerIdle";
	}

	return "";
}