#include "LwaxanaTroi.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "GameApp.h"

LwaxanaTroi::LwaxanaTroi()
{
	this->SetName("Lwaxana");
}

/*virtual*/ LwaxanaTroi::~LwaxanaTroi()
{
}

/*virtual*/ uint64_t LwaxanaTroi::GetAdditionalUserFlagsForCollisionShape()
{
	return SHAPE_FLAG_TALKER;
}

/*virtual*/ bool LwaxanaTroi::Setup()
{
	std::string modelFile = "Models/LwaxanaTroi/LwaxanaTroi.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile, &this->restartLocation, &this->restartOrientation));

	if (!Biped::Setup())
		return false;

	return true;
}

/*virtual*/ bool LwaxanaTroi::Shutdown()
{
	Biped::Shutdown();
	return true;
}

/*virtual*/ void LwaxanaTroi::AdjustFacingDirection(double deltaTime)
{
	Imzadi::Reference<Imzadi::Entity> entity;
	if (Imzadi::Game::Get()->FindEntityByName("Deanna", entity))
	{
		Imzadi::Transform deannaTransform;
		entity->GetTransform(deannaTransform);

		Imzadi::Transform lwaxanaTransform;
		this->GetTransform(lwaxanaTransform);

		Imzadi::Vector3 direction = (deannaTransform.translation - lwaxanaTransform.translation).Normalized();
		Imzadi::Vector3 xAxis, yAxis, zAxis;
		yAxis.SetComponents(0.0, 1.0, 0.0);
		zAxis = (-direction).RejectedFrom(yAxis).Normalized();
		xAxis = yAxis.Cross(zAxis);
		lwaxanaTransform.matrix.SetColumnVectors(xAxis, yAxis, zAxis);

		this->SetTransform(lwaxanaTransform);
	}
}

/*virtual*/ bool LwaxanaTroi::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Biped::Tick(tickPass, deltaTime))
		return false;

	return true;
}

/*virtual*/ void LwaxanaTroi::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	Biped::IntegrateVelocity(acceleration, deltaTime);

	this->velocity.x = 0.0;
	this->velocity.z = 0.0;
}

/*virtual*/ std::string LwaxanaTroi::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "LwaxanaTroiIdle";
	}

	return "";
}