#pragma once

#include "Entities/Level.h"
#include "Audio/System.h"

class GameLevel : public Imzadi::Level
{
public:
	GameLevel();
	virtual ~GameLevel();

	virtual bool Setup() override;
	virtual bool SetupWithLevelData(Imzadi::LevelData* levelData) override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual Imzadi::Biped* SpawnMainCharacter() override;
	virtual void SpawnNPC(const Imzadi::LevelData::NPC* npc) override;
	virtual void AdjustCollisionWorldExtents(Imzadi::AxisAlignedBoundingBox& collisionWorldBox) override;

private:
	void HandleLevel6MIDIEvent(const Imzadi::MidiSongEvent* event);
};