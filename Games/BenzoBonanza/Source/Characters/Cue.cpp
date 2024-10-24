#include "Cue.h"
#include "GameApp.h"

Cue::Cue()
{
	this->SetName("Cue");
}

/*virtual*/ Cue::~Cue()
{
}

/*virtual*/ bool Cue::Setup()
{
	std::string modelFile = "Models/Cue/Cue.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	return true;
}

/*virtual*/ bool Cue::Shutdown()
{
	Character::Shutdown();

	return true;
}

/*virtual*/ void Cue::ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Imzadi::Vector3(0.0, 2.5, 0.0));
	capsule->SetVertex(1, Imzadi::Vector3(0.0, 6.5, 0.0));
	capsule->SetRadius(2.5);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY | SHAPE_FLAG_TALKER | SHAPE_FLAG_BADDY);
}

/*virtual*/ bool Cue::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	return true;
}

/*virtual*/ std::string Cue::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "CueIdle";
	case Imzadi::Biped::AnimType::RUN:
		return "CueRun";
	}

	return "";
}

/*virtual*/ void Cue::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	Character::IntegrateVelocity(acceleration, deltaTime);
}