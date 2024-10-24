#pragma once

#include "Character.h"
#include "Math/Random.h"
#include "Assets/NavGraph.h"
#include "Characters/Alice.h"

/**
 * Again, to avoid copyright infringement, this character parodies
 * the character "Q" from StarTrek(TM).  But I shouldn't even say that.
 * It's a character that just coincidentally looks like that character.
 * 
 * This character is designed to operate on a nav-graph, if the level has one.
 * It will use the nav-graph to travel, as fast as it can, toward the player,
 * to try to kill the player.
 */
class Cue : public Character
{
public:
	Cue();
	virtual ~Cue();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual std::string GetAnimName(Imzadi::Biped::AnimType animType) override;
	virtual void ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule) override;
	virtual void IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime) override;
	virtual bool OnBipedDied() override;

private:

	void PersuePlayer();

	enum Disposition
	{
		NONE,
		STAY_STILL_FOR_A_BIT,
		PERSUE_PLAYER_MERCILESSLY,
		TELEPORT
	};

	Disposition disposition;
	double remainingStillTimeSeconds;
	Imzadi::Random random;
	Imzadi::Reference<Imzadi::NavGraph> navGraph;
	Imzadi::Reference<Alice> alice;
	const Imzadi::NavGraph::Node* aliceNode;
	const Imzadi::NavGraph::Node* cueNode;
	std::list<int> runPathList;
	bool debugDrawRunPath;
	double runSpeed;
	Imzadi::Vector3 desiredVelocity;
};