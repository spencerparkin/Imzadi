#pragma once

#include "Entities/Biped.h"
#include "Abilities.h"

class Character : public Imzadi::Biped
{
public:
	Character();
	virtual ~Character();

	enum ControlMode
	{
		INTERNAL,		///< The character is being controlled internally.
		EXTERNAL		///< The character is being controlled externally.
	};

	void SetControlMode(ControlMode controlMode);
	ControlMode GetControlMode() const;

	virtual bool HangingOnToZipLine();
	virtual void OnReleasedFromZipLine();
	virtual std::string GetZipLineAnimationName();
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

protected:

	Imzadi::Reference<Abilities> abilities;
	ControlMode controlMode;
};