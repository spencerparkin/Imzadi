#include "GameLevel.h"
#include "GameApp.h"
#include "Characters/DeannaTroi.h"
#include "Characters/LwaxanaTroi.h"
#include "Characters/Borg.h"
#include "Pickup.h"
#include "Assets/GameLevelData.h"
#include "Entities/ZipLineEntity.h"
#include "Entities/WarpTunnel.h"
#include "Audio/System.h"
#include "Math/Interval.h"
#include "RenderObjects/HUDRenderObject.h"
#include "Assets/WarpTunnelData.h"

GameLevel::GameLevel()
{
}

/*virtual*/ GameLevel::~GameLevel()
{
}

/*virtual*/ Imzadi::Biped* GameLevel::SpawnMainCharacter()
{
	return Imzadi::Game::Get()->SpawnEntity<DeannaTroi>();
}

/*virtual*/ bool GameLevel::Setup()
{
	if (!Level::Setup())
		return false;

	if (this->GetLevelName() == "Level1")
	{
		auto lwaxana = Imzadi::Game::Get()->SpawnEntity<LwaxanaTroi>();
		lwaxana->SetRestartLocation(Imzadi::Vector3(-6.814, 1.6, -105.338));
		lwaxana->SetRestartOrientation(Imzadi::Quaternion());
	}
	else if (this->GetLevelName() == "Level6")
	{
		Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("MIDI", new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
			this->HandleLevel6MIDIEvent(dynamic_cast<const Imzadi::MidiSongEvent*>(event));
		}));
	}

	Imzadi::AudioSystem* audioSystem = Imzadi::Game::Get()->GetAudioSystem();
	audioSystem->AddAmbientSound({ "BlowingWind", "HowlingWind" }, Imzadi::Interval(25, 29), true, 0.1f);
	audioSystem->AddAmbientSound({ "OwlSound", "WindChimes" }, Imzadi::Interval(50, 100), false, 1.0f);

	auto hud = new HUDRenderObject();
	hud->SetFont("UbuntuMono_R");
	Imzadi::Game::Get()->GetScene()->AddRenderObject(hud);
	return true;
}

/*virtual*/ bool GameLevel::SetupWithLevelData(Imzadi::LevelData* levelData)
{
	if (!Level::SetupWithLevelData(levelData))
		return false;

	auto gameLevelData = dynamic_cast<GameLevelData*>(levelData);
	if (!gameLevelData)
		return false;

	for (int i = 0; i < (signed)gameLevelData->GetZipLineArray().size(); i++)
	{
		ZipLine* zipLine = const_cast<ZipLine*>(gameLevelData->GetZipLineArray()[i].Get());
		ZipLineEntity* zipLineEntity = Imzadi::Game::Get()->SpawnEntity<ZipLineEntity>();
		zipLineEntity->SetZipLine(zipLine);
	}

	return true;
}

/*virtual*/ bool GameLevel::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Level::Tick(tickPass, deltaTime))
		return false;

	auto game = (GameApp*)Imzadi::Game::Get();
	game->GetDialogSystem()->Tick();
	return true;
}

/*virtual*/ void GameLevel::SpawnNPC(const Imzadi::LevelData::NPC* npc)
{
	Imzadi::Game* game = Imzadi::Game::Get();

	if (npc->type == "borg")
	{
		Character* character = nullptr;

		if (npc->type == "borg")
			character = game->SpawnEntity<Borg>();

		if (character)
		{
			character->SetRestartLocation(npc->startPosition);
			character->SetRestartOrientation(npc->startOrientation);
			character->SetCanRestart(false);
		}
	}
	else if (npc->type == "heart" || npc->type == "speed_boost" || npc->type == "song")
	{
		Pickup* pickup = nullptr;

		if (npc->type == "heart")
			pickup = game->SpawnEntity<ExtraLifePickup>();
		else if (npc->type == "speed_boost")
			pickup = game->SpawnEntity<SpeedBoostPickup>();
		else if (npc->type == "song")
			pickup = game->SpawnEntity<SongPickup>();

		if (pickup)
		{
			pickup->SetTransform(Imzadi::Transform(npc->startOrientation, npc->startPosition));
			pickup->Configure(npc->configMap);
		}
	}
}

/*virtual*/ void GameLevel::AdjustCollisionWorldExtents(Imzadi::AxisAlignedBoundingBox& collisionWorldBox)
{
	Level::AdjustCollisionWorldExtents(collisionWorldBox);

	if (this->GetLevelName() == "Level6")
	{
		// We do this to accomedate the warp-tunnels.
		collisionWorldBox.Scale(1.5, 1.0, 1.5);
	}
}

void GameLevel::HandleLevel6MIDIEvent(const Imzadi::MidiSongEvent* event)
{
	std::vector<Imzadi::Entity*> entityArray;
	if (!Imzadi::Game::Get()->FindAllEntitiesWithName("Tunnel2", entityArray))
		return;

	Imzadi::Reference<Imzadi::WarpTunnel> warpTunnel;
	bool warpTunnelFound = false;
	for (auto entity : entityArray)
	{
		warpTunnel.SafeSet(entity);
		if (warpTunnel.Get())
		{
			const Imzadi::WarpTunnelData* data = warpTunnel->GetData();
			const Imzadi::WarpTunnelData::PortBind* portBind = data->GetPortBind(0);
			if (portBind && portBind->foreignMesh == "Chamber1" && portBind->foreignPort == "Port2")
			{
				warpTunnelFound = true;
				break;
			}
		}
	}

	if (warpTunnelFound)
	{
		Imzadi::WarpTunnelData* data = warpTunnel->GetData();
		Imzadi::WarpTunnelData::PortBind* portBind = data->GetPortBind(1);
		if (portBind)
		{
			switch (event->type)
			{
				case Imzadi::MidiSongEvent::SONG_STARTED:
				{
					portBind->foreignMesh = "FinalChamber";
					portBind->foreignPort = "Port24";
					break;
				}
				case Imzadi::MidiSongEvent::SONG_FINISHED:
				{
					portBind->foreignMesh = "Chamber2";
					portBind->foreignPort = "Port13";
					break;
				}
			}
		}
	}
}