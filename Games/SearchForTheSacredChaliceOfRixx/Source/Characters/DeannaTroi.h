#pragma once

#include "Entities/Biped.h"

class DeannaTroi : public Imzadi::Biped
{
public:
	DeannaTroi();
	virtual ~DeannaTroi();

	virtual bool Setup() override;
	virtual bool Shutdown(bool gameShuttingDown) override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual void AccumulateForces(Imzadi::Vector3& netForce) override;
	virtual void IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime) override;
	virtual void Reset() override;

private:
	uint32_t cameraHandle;
	double maxMoveSpeed;
};