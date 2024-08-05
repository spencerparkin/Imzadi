#include "Character.h"
#include "Collision/Command.h"
#include "Game.h"

Character::Character()
{
	this->controlMode = ControlMode::INTERNAL;
}

/*virtual*/ Character::~Character()
{
}

void Character::SetControlMode(ControlMode controlMode)
{
	this->controlMode = controlMode;
}

Character::ControlMode Character::GetControlMode() const
{
	return this->controlMode;
}

/*virtual*/ bool Character::HangingOnToZipLine()
{
	return false;
}

/*virtual*/ void Character::OnReleasedFromZipLine()
{
	Imzadi::Transform objectToWorld;
	this->GetTransform(objectToWorld);

	this->objectToPlatform = objectToWorld;
	this->platformToWorld.SetIdentity();
	this->inContactWithGround = false;
}

/*virtual*/ bool Character::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (this->controlMode == ControlMode::INTERNAL)
		return Biped::Tick(tickPass, deltaTime);

	return true;
}