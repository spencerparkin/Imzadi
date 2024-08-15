#pragma once

#include "Character.h"

class Borg : public Character
{
public:
	Borg();
	virtual ~Borg();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual uint64_t GetAdditionalUserFlagsForCollisionShape() override;
	virtual std::string GetAnimName(Imzadi::Biped::AnimType animType) override;
};