#pragma once

#include "Entities/Hero.h"

class DeannaTroi : public Imzadi::Hero
{
public:
	DeannaTroi();
	virtual ~DeannaTroi();

	virtual bool Setup() override;
	virtual bool Shutdown(bool gameShuttingDown) override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
};