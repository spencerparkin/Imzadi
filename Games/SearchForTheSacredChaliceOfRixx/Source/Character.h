#pragma once

#include "Entities/Biped.h"

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

	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

protected:

	ControlMode controlMode;
};