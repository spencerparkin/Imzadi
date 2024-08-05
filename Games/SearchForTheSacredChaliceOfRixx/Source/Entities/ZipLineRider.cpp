#include "ZipLineRider.h"
#include "Character.h"

ZipLineRider::ZipLineRider()
{
}

/*virtual*/ ZipLineRider::~ZipLineRider()
{
}

/*virtual*/ bool ZipLineRider::Setup()
{
	if (!Entity::Setup())
		return false;

	if (!this->character || !this->zipLine)
		return false;

	this->character->SetControlMode(Character::ControlMode::EXTERNAL);
	this->character->SetVelocity(Imzadi::Vector3(0.0, 0.0, 0.0));

	Imzadi::Transform objectToWorld;
	this->character->GetTransform(objectToWorld);
	this->startingPoint = objectToWorld.translation;

	return true;
}

/*virtual*/ bool ZipLineRider::Shutdown()
{
	Entity::Shutdown();

	return true;
}

/*virtual*/ bool ZipLineRider::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	Imzadi::Transform objectToWorld;
	this->character->GetTransform(objectToWorld);

	double distanceTraveled = (objectToWorld.translation - this->startingPoint).Length();

	if (distanceTraveled >= this->zipLine->GetLineSegment().Length() || !this->character->HangingOnToZipLine())
	{
		// Note that the character inherits velocity gained while having zipped along the zip-line.
		this->character->SetControlMode(Character::ControlMode::INTERNAL);
		this->character->OnReleasedFromZipLine();
		return false;
	}

	// TODO: My math might be wrong here, because I swear that the character is going
	//       faster while zip-lining than how fast they go once they're released from
	//       the zip-line.  I'm not sure what I'm doing wrong.
	Imzadi::Vector3 downVector(0.0, -1.0, 0.0);
	Imzadi::Vector3 gravityForce = downVector * this->character->GetMass() * Imzadi::Game::Get()->GetGravity();
	Imzadi::Vector3 zipLineForce = gravityForce.ProjectedOnto(this->zipLine->GetLineSegment().GetDelta().Normalized());
	Imzadi::Vector3 velocity = this->character->GetVelocity();
	Imzadi::Vector3 frictionForce = -velocity * this->zipLine->GetFrictionFactor();
	Imzadi::Vector3 netForce = zipLineForce + frictionForce;
	Imzadi::Vector3 acceleration = netForce / this->character->GetMass();
	
	velocity += acceleration * deltaTime;
	this->character->SetVelocity(velocity);
	
	objectToWorld.translation += velocity * deltaTime;
	this->character->SetTransform(objectToWorld);

	return true;
}

void ZipLineRider::SetCharacter(Character* character)
{
	this->character.Set(character);
}

Character* ZipLineRider::GetCharacter()
{
	return this->character.Get();
}

void ZipLineRider::SetZipLine(ZipLine* zipLine)
{
	this->zipLine.Set(zipLine);
}

ZipLine* ZipLineRider::GetZipLine()
{
	return this->zipLine.Get();
}