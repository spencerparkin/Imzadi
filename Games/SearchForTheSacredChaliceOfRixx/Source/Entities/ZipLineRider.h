#pragma once

#include "Entity.h"
#include "Assets/ZipLine.h"

class Character;

class ZipLineRider : public Imzadi::Entity
{
public:
	ZipLineRider();
	virtual ~ZipLineRider();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

	void SetCharacter(Character* character);
	Character* GetCharacter();

	void SetZipLine(ZipLine* zipLine);
	ZipLine* GetZipLine();

private:
	Imzadi::Reference<Character> character;
	Imzadi::Reference<ZipLine> zipLine;
	Imzadi::Vector3 startingPoint;
};