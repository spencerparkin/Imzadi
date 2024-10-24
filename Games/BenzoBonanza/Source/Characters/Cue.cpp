#include "Cue.h"
#include "GameApp.h"
#include "Entities/Level.h"
#include "RenderObjects/DebugLines.h"

Cue::Cue()
{
	this->SetName("Cue");
	this->disposition = Disposition::NONE;
	this->random.SetSeed(int(uintptr_t(this)));
	this->aliceNode = nullptr;
	this->cueNode = nullptr;
	this->remainingStillTimeSeconds = 0.0;
	this->debugDrawRunPath = false;
	this->runSpeed = 0.0;
	this->canRestart = true;
}

/*virtual*/ Cue::~Cue()
{
}

/*virtual*/ bool Cue::Setup()
{
	std::string modelFile = "Models/Cue/Cue.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	return true;
}

/*virtual*/ bool Cue::Shutdown()
{
	Character::Shutdown();

	this->aliceNode = nullptr;
	this->cueNode = nullptr;
	this->navGraph.Reset();

	return true;
}

/*virtual*/ void Cue::ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Imzadi::Vector3(0.0, 2.5, 0.0));
	capsule->SetVertex(1, Imzadi::Vector3(0.0, 6.5, 0.0));
	capsule->SetRadius(2.5);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY | SHAPE_FLAG_TALKER | SHAPE_FLAG_BADDY);
}

/*virtual*/ bool Cue::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	if (tickPass == Imzadi::TickPass::PARALLEL_WORK)
	{
		switch (this->disposition)
		{
			case Disposition::NONE:
			{
				this->desiredVelocity.SetComponents(0.0, 0.0, 0.0);
				this->remainingStillTimeSeconds = this->random.InRange(3.0, 5.0);
				this->disposition = Disposition::STAY_STILL_FOR_A_BIT;
				break;
			}
			case Disposition::STAY_STILL_FOR_A_BIT:
			{
				this->remainingStillTimeSeconds -= deltaTime;
				if (this->remainingStillTimeSeconds <= 0.0)
				{
					this->remainingStillTimeSeconds = 0.0;
					this->disposition = Disposition::PERSUE_PLAYER_MERCILESSLY;
					this->runSpeed = this->random.InRange(20.0, 30.0);
				}
				break;
			}
			case Disposition::PERSUE_PLAYER_MERCILESSLY:
			{
				this->PersuePlayer();
				break;
			}
			case Disposition::TELEPORT:
			{
				IMZADI_ASSERT(this->navGraph.Get());
				const Imzadi::NavGraph::Node* node = this->navGraph->GetRandomNode(this->random);
				this->inContactWithGround = false;
				this->objectToPlatform.SetIdentity();
				this->platformToWorld.translation = node->location;
				this->platformToWorld.matrix.SetIdentity();
				this->desiredVelocity.SetComponents(0.0, 0.0, 0.0);
				this->disposition = Disposition::NONE;
				break;
			}
		}

		if (this->debugDrawRunPath && this->navGraph.Get() && this->cueNode)
		{
			Imzadi::DebugLines* debugLines = Imzadi::Game::Get()->GetDebugLines();
			if (debugLines)
			{
				const Imzadi::NavGraph::Node* node = this->cueNode;
				for(int i : this->runPathList)
				{
					const Imzadi::NavGraph::Node* nextNode = node->GetAdjacentNode(i);
					if (!nextNode)
						break;
					
					Imzadi::DebugLines::Line line;
					line.color.SetComponents(0.0, 1.0, 0.0);
					line.segment.point[0] = node->location;
					line.segment.point[1] = nextNode->location;
					debugLines->AddLine(line);

					node = nextNode;
				}
			}
		}
	}

	return true;
}

void Cue::PersuePlayer()
{
	// Make sure we have a cached pointer to the nav-graph.
	if (!this->navGraph.Get())
	{
		std::vector<Imzadi::Level*> foundLevelsArray;
		Imzadi::Game::Get()->FindAllEntitiesOfType<Imzadi::Level>(foundLevelsArray);
		if (foundLevelsArray.size() == 1)
		{
			Imzadi::Level* level = foundLevelsArray[0];
			this->navGraph = level->GetNavGraph();
		}

		if (!this->navGraph.Get())
			return;
	}

	// Make sure we have a cached pointer to the player character.
	if (!this->alice.Get())
	{
		Imzadi::Reference<Imzadi::Entity> foundEntity;
		Imzadi::Game::Get()->FindEntityByName("Alice", foundEntity);
		this->alice.SafeSet(foundEntity.Get());
		if (!this->alice.Get())
			return;
	}

	// Locate our position in world space.
	Imzadi::Transform cueObjectToWorld;
	this->GetTransform(cueObjectToWorld);

	// Locate Alice in the nav-graph to the nearest path.
	Imzadi::Transform aliceObjectToWorld;
	this->alice->GetTransform(aliceObjectToWorld);
	int terminalIndex = -1;
	const Imzadi::NavGraph::Path* path = this->navGraph->FindNearestPath(aliceObjectToWorld.translation, &terminalIndex);
	if (!path)
	{
		this->desiredVelocity.SetComponents(0.0, 0.0, 0.0);
		return;
	}

	// Has she changed her last known position?
	const Imzadi::NavGraph::Node* node = path->terminalNode[terminalIndex];
	if (node != this->aliceNode)
	{
		// Yes.  Calculate a shortest-path from where we are to where she is.
		this->aliceNode = node;
		const Imzadi::NavGraph::Path* path = this->navGraph->FindNearestPath(cueObjectToWorld.translation, &terminalIndex);
		IMZADI_ASSERT(path);
		this->cueNode = path->terminalNode[terminalIndex];
		bool pathFound = this->navGraph->FindShortestPath(this->cueNode, this->aliceNode, this->runPathList);
		if (!pathFound)
		{
			this->disposition = Disposition::NONE;
			return;
		}
	}

	// Are we occupying the same nav-graph node with Alice?  (Or, are we within a certain distance of her?)
	double distanceToAlice = (aliceObjectToWorld.translation - cueObjectToWorld.translation).Length();
	const double minDistanceToAlice = 10.0;
	if (distanceToAlice < minDistanceToAlice || this->cueNode == this->aliceNode)
	{
		// Yes.  Run her down!
		const double killDistance = 2.0;
		if (distanceToAlice > killDistance)
		{
			this->desiredVelocity = (aliceObjectToWorld.translation - cueObjectToWorld.translation).Normalized() * this->runSpeed;
		}
		else
		{
			this->desiredVelocity.SetComponents(0.0, 0.0, 0.0);
			this->aliceNode = nullptr;
			this->cueNode = nullptr;
			this->runPathList.clear();
			this->disposition = Disposition::TELEPORT;
		}
	}
	else
	{
		// No.  Run to the next nav-graph node in our path.
		IMZADI_ASSERT(this->runPathList.size() > 0);
		int i = *this->runPathList.begin();
		const Imzadi::NavGraph::Node* nextNode = this->cueNode->GetAdjacentNode(i);
		IMZADI_ASSERT(nextNode);
		
		// Are we close enough to the next node?
		double distanceToNextNode = (cueObjectToWorld.translation - nextNode->location).Length();
		const double minDisatnceToNextNode = 5.0;
		if (distanceToNextNode <= minDisatnceToNextNode)
		{
			// Yes.  Advance to the next node.
			this->cueNode = nextNode;
			this->runPathList.pop_front();
		}
		else
		{
			// No.  Set our velocity to take us to the next node.
			// Note that the world-space and platform-space velocity vectors should be the same.
			this->desiredVelocity = (nextNode->location - cueObjectToWorld.translation).Normalized() * this->runSpeed;
		}
	}
}

/*virtual*/ bool Cue::OnBipedDied()
{
	this->aliceNode = nullptr;
	this->cueNode = nullptr;
	this->runPathList.clear();
	this->disposition = Disposition::NONE;

	return Character::OnBipedDied();
}

/*virtual*/ std::string Cue::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "CueIdle";
	case Imzadi::Biped::AnimType::RUN:
		return "CueRun";
	}

	return "";
}

/*virtual*/ void Cue::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	// Don't stomp the Y value so that we're subject to gravity.
	this->velocity.x = this->desiredVelocity.x;
	this->velocity.z = this->desiredVelocity.z;

	Character::IntegrateVelocity(acceleration, deltaTime);
}