#pragma once

#include "Entity.h"
#include "Assets/ZipLine.h"

class ZipLineEntity : public Imzadi::Entity
{
public:
	ZipLineEntity();
	virtual ~ZipLineEntity();

	void SetZipLine(ZipLine* givenZipLine);

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

private:
	Imzadi::Reference<ZipLine> zipLine;
};