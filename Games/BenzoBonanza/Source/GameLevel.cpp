#include "GameLevel.h"
#include "GameApp.h"
#include "Characters/Alice.h"
#include "Characters/Spencer.h"
#include "Characters/Borggy.h"
#include "Characters/Bob.h"
#include "Pickup.h"
#include "Assets/GameLevelData.h"
#include "Entities/ZipLineEntity.h"
#include "Entities/WarpTunnel.h"
#include "Entities/RubiksCubie.h"
#include "Entities/SlidingDoor.h"
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
	return Imzadi::Game::Get()->SpawnEntity<Alice>();
}

/*virtual*/ bool GameLevel::Setup()
{
	if (!Level::Setup())
		return false;

	if (this->GetLevelName() == "Level1")
	{
		auto spencer = Imzadi::Game::Get()->SpawnEntity<Spencer>();
		spencer->SetRestartLocation(Imzadi::Vector3(-6.814, 1.6, -105.338));
		spencer->SetRestartOrientation(Imzadi::Quaternion());
	}
	else if (this->GetLevelName() == "Level6")
	{
		Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("MIDI", Imzadi::EventListenerType::TRANSITORY, new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
			this->HandleLevel6MIDIEvent(dynamic_cast<const Imzadi::MidiSongEvent*>(event));
		}));
	}

	Imzadi::AudioSystem* audioSystem = Imzadi::Game::Get()->GetAudioSystem();
	audioSystem->AddAmbientSound({ "BlowingWind", "HowlingWind" }, Imzadi::Interval(25, 29), true, 0.1f);
	audioSystem->AddAmbientSound({ "OwlSound", "WindChimes" }, Imzadi::Interval(50, 100), false, 1.0f);

	auto game = (GameApp*)Imzadi::Game::Get();
	auto hud = new HUDRenderObject();
	hud->SetFont("UbuntuMono_R");
	game->GetScene()->AddRenderObject(hud);
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

	for (const std::string& cubieFile : gameLevelData->GetCubieFilesArray())
	{
		auto rubiksCubie = Imzadi::Game::Get()->SpawnEntity<RubiksCubie>();
		rubiksCubie->SetMovingPlatformFile(cubieFile);
	}

	for (const std::string& doorFile : gameLevelData->GetDoorFilesArray())
	{
		auto door = Imzadi::Game::Get()->SpawnEntity<SlidingDoor>();
		door->SetMovingPlatformFile(doorFile);
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

	if (npc->type == "bob" ||
		npc->type == "riker")
	{
		Character* character = nullptr;

		if (npc->type == "borg")
			character = game->SpawnEntity<Borggy>();
		else if (npc->type == "bob")
			character = game->SpawnEntity<Bob>();

		if (character)
		{
			character->SetRestartLocation(npc->startPosition);
			character->SetRestartOrientation(npc->startOrientation);
			character->SetCanRestart(false);
		}
	}
	else if (
		npc->type == "heart" ||
		npc->type == "speed_boost" ||
		npc->type == "song" ||
		npc->type == "key")
	{
		Pickup* pickup = nullptr;

		if (npc->type == "heart")
			pickup = game->SpawnEntity<ExtraLifePickup>();
		else if (npc->type == "speed_boost")
			pickup = game->SpawnEntity<SpeedBoostPickup>();
		else if (npc->type == "song")
			pickup = game->SpawnEntity<SongPickup>();
		else if (npc->type == "key")
			pickup = game->SpawnEntity<KeyPickup>();

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