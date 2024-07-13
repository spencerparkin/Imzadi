#pragma once

#include "Entities/Biped.h"

class LwaxanaTroi : public Imzadi::Biped
{
public:
	LwaxanaTroi();
	virtual ~LwaxanaTroi();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual void IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime) override;
	virtual std::string GetAnimName(Imzadi::Biped::AnimType animType) override;
	virtual void AdjustFacingDirection(double deltaTime) override;
	virtual uint64_t GetAdditionalUserFlagsForCollisionShape() override;
};