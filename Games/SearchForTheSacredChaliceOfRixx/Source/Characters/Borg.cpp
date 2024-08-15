#include "Borg.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "GameApp.h"
#include <format>

Borg::Borg()
{
	static int count = 0;
	this->SetName(std::format("Borg{}", count++));
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

/*virtual*/ bool Borg::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	return true;
}

/*virtual*/ uint64_t Borg::GetAdditionalUserFlagsForCollisionShape()
{
	return SHAPE_FLAG_TALKER;
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