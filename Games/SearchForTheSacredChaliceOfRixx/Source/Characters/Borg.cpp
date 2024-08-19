#include "Borg.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "GameApp.h"

Borg::Borg()
{
	this->SetName("Borg");
}

/*virtual*/ Borg::~Borg()
{
}

/*virtual*/ bool Borg::Setup()
{
	std::string modelFile = "Models/Borg/Borg.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	return true;
}

/*virtual*/ bool Borg::Shutdown()
{
	Character::Shutdown();

	return true;
}

/*virtual*/ void Borg::ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Imzadi::Vector3(0.0, 2.5, 0.0));
	capsule->SetVertex(1, Imzadi::Vector3(0.0, 6.5, 0.0));
	capsule->SetRadius(2.5);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY | SHAPE_FLAG_TALKER);
}

/*virtual*/ bool Borg::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	return true;
}

/*virtual*/ std::string Borg::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "BorgIdle";
	case Imzadi::Biped::AnimType::RUN:
		return "BorgRun";
	}

	return "";
}