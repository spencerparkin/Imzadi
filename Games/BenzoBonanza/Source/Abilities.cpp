#include "Abilities.h"

//---------------------------------- Abilities ----------------------------------

Abilities::Abilities()
{
}

/*virtual*/ Abilities::~Abilities()
{
}

void Abilities::Tick(double deltaTime)
{
	std::vector<std::string> expiredAbilityArray;

	for (auto pair : this->abilityMap)
	{
		Ability* ability = pair.second;
		ability->Tick(deltaTime);

		if (ability->lifeTimeSeconds == 0.0)
			expiredAbilityArray.push_back(pair.first);
	}

	for (const std::string& abilityName : expiredAbilityArray)
		this->SetAbility(abilityName, nullptr, true);
}

void Abilities::Clear()
{
	this->abilityMap.clear();
}

bool Abilities::SetAbility(const std::string& name, Ability* ability, bool canReplaceExisting /*= true*/)
{
	auto existingAbility = const_cast<Ability*>(this->FindAbility(name));
	if (existingAbility && !canReplaceExisting)
		return false;

	if (!ability)
	{
		if (existingAbility && existingAbility->overrideAbility.Get())
		{
			Imzadi::Reference<Ability> overrideAbility = existingAbility->overrideAbility;
			this->abilityMap.erase(name);
			this->abilityMap.insert(std::pair(name, overrideAbility));
		}
		else if (existingAbility)
		{
			this->abilityMap.erase(name);
		}
	}
	else
	{
		if (existingAbility && existingAbility->overrideAbility.Get())
			ability->overrideAbility = existingAbility->overrideAbility;
		
		if (existingAbility)
			this->abilityMap.erase(name);

		this->abilityMap.insert(std::pair(name, ability));
	}

	return true;
}

bool Abilities::OverrideAbility(const std::string& name, Ability* ability, double lifeTimeSeconds)
{
	if (!ability)
		return false;

	auto existingAbility = const_cast<Ability*>(this->FindAbility(name));
	if (!existingAbility)
		return false;

	ability->overrideAbility = existingAbility;
	ability->lifeTimeSeconds = lifeTimeSeconds;
	this->abilityMap.erase(name);
	this->abilityMap.insert(std::pair(name, ability));
	return true;
}

bool Abilities::UseAbility(const std::string& name, Character* character) const
{
	auto ability = const_cast<Ability*>(this->FindAbility(name));
	if (!ability)
		return false;

	return ability->Use(character);
}

bool Abilities::GetAbilityValue(const std::string& name, double& value) const
{
	const Ability* ability = this->FindAbility(name);
	if (!ability)
		return false;

	return ability->GetValue(value);
}

bool Abilities::GetAbilityValue(const std::string& name, Imzadi::Vector3& value) const
{
	const Ability* ability = this->FindAbility(name);
	if (!ability)
		return false;

	return ability->GetValue(value);
}

const Ability* Abilities::FindAbility(const std::string& name) const
{
	AbilityMap::const_iterator iter = this->abilityMap.find(name);
	if (iter == this->abilityMap.end())
		return nullptr;

	return iter->second;
}

//---------------------------------- Ability ----------------------------------

Ability::Ability()
{
	this->lifeTimeSeconds = std::numeric_limits<double>::max();
}

/*virtual*/ Ability::~Ability()
{
}

/*virtual*/ void Ability::Tick(double deltaTime)
{
	if (this->lifeTimeSeconds != std::numeric_limits<double>::max())
	{
		this->lifeTimeSeconds -= deltaTime;
		if (this->lifeTimeSeconds < 0.0)
			this->lifeTimeSeconds = 0.0;
	}
}

/*virtual*/ bool Ability::GetValue(double& value) const
{
	return false;
}

/*virtual*/ bool Ability::GetValue(Imzadi::Vector3& value) const
{
	return false;
}

/*virtual*/ bool Ability::Use(Character* character)
{
	return false;
}

//---------------------------------- FloatAbility ----------------------------------

FloatAbility::FloatAbility()
{
	this->value = 0.0;
}

FloatAbility::FloatAbility(double value)
{
	this->value = value;
}

/*virtual*/ FloatAbility::~FloatAbility()
{
}

/*virtual*/ bool FloatAbility::GetValue(double& value) const
{
	value = this->value;
	return true;
}

//---------------------------------- VectorAbility ----------------------------------

VectorAbility::VectorAbility()
{
	this->vector.SetComponents(0.0, 0.0, 0.0);
}

/*virtual*/ VectorAbility::~VectorAbility()
{
}

/*virtual*/ bool VectorAbility::GetValue(Imzadi::Vector3& value) const
{
	value = this->vector;
	return true;
}