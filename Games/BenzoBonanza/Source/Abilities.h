#pragma once

#include "Reference.h"
#include "Math/Vector3.h"
#include <unordered_map>

class Ability;
class Character;

/**
 * This is an abstraction for character abilities.
 * It's basically a container for such things.  An ability
 * is a named thing with associated values.  These can
 * be queried when needed to perform an action.  Also, some
 * abilities can perminantly replace, or temporarily override
 * other abilities.
 * 
 * Admittedly, none of this is very well thought-out, really.
 * But I imagine that a typical game-level API has something
 * like this.
 */
class Abilities : public Imzadi::ReferenceCounted
{
public:
	Abilities();
	virtual ~Abilities();

	void Tick(double deltaTime);
	void Clear();
	bool SetAbility(const std::string& name, Ability* ability, bool canReplaceExisting = true);
	bool OverrideAbility(const std::string& name, Ability* ability, double lifeTimeSeconds);
	bool UseAbility(const std::string& name, Character* character) const;
	bool GetAbilityValue(const std::string& name, double& value) const;
	bool GetAbilityValue(const std::string& name, Imzadi::Vector3& value) const;
	const Ability* FindAbility(const std::string& name) const;

private:
	typedef std::unordered_map<std::string, Imzadi::Reference<Ability>> AbilityMap;
	AbilityMap abilityMap;
};

/**
 * Define the interface for all ability derivatives.
 * Also, make sure every ability has a life-time and
 * other things common among all such things.
 */
class Ability : public Imzadi::ReferenceCounted
{
	friend class Abilities;

public:
	Ability();
	virtual ~Ability();

	/**
	 * Run-down the clock on this ability's life-time.  Of course,
	 * a derived class could override this method, either replacing
	 * or augmenting this functionality.  E.g., maybe, when activated,
	 * you get a shield, but it's only active for the life-time of
	 * the ability.
	 */
	virtual void Tick(double deltaTime);

	/**
	 * Get the value of this ability as a floating-point number.
	 * E.g., this might be jump-force or run-speed or something
	 * like that.
	 */
	virtual bool GetValue(double& value) const;

	/**
	 * Get the value of this ability as a vector.
	 */
	virtual bool GetValue(Imzadi::Vector3& value) const;

	/**
	 * If using the ability is something that can be done by
	 * calling a function, then this is where you'd do it.
	 * Some abilities, however, are not used this way.  Rather,
	 * when a normal action is taken, the parameters of that
	 * action are taken from the ability.
	 * 
	 * Suppose this is an ability you can only use a finite
	 * number of times.  The derived class could decrement a
	 * count on each usage.  Once at zero, just set the life-time
	 * of the ability to zero and it will get destroyed automatically.
	 * 
	 * @param[in] character This is character using the ability, presumably the one who owns the ability.
	 */
	virtual bool Use(Character* character);

protected:
	Imzadi::Reference<Ability> overrideAbility;
	double lifeTimeSeconds;		///< A life-time of max-double means infinite.
};

/**
 * These are generic abilities defined by a single floating-point number.
 */
class FloatAbility : public Ability
{
public:
	FloatAbility();
	FloatAbility(double value);
	virtual ~FloatAbility();

	bool GetValue(double& value) const override;

public:
	double value;
};

/**
 * These are generic abilities defined by a single vector.
 */
class VectorAbility : public Ability
{
public:
	VectorAbility();
	virtual ~VectorAbility();

	bool GetValue(Imzadi::Vector3& value) const override;

public:
	Imzadi::Vector3 vector;
};