#include "DialogSystem.h"
#include "Assets/DialogData.h"
#include "CustomAssetCache.h"
#include "GameApp.h"
#include "Entity.h"
#include "Log.h"

//--------------------------------- DialogSystem ---------------------------------

DialogSystem::DialogSystem()
{
	this->currentDialogSequencePosition = -1;
}

/*virtual*/ DialogSystem::~DialogSystem()
{
}

bool DialogSystem::Initialize()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	auto assetCache = game->GetAssetCache();

	Imzadi::Reference<Imzadi::Asset> asset;
	if (!assetCache->LoadAsset("Dialog/Dialog.dialog", asset))
	{
		IMZADI_LOG_ERROR("Failed to load dialog data asset.");
		return false;
	}

	this->dialogData.SafeSet(asset.Get());
	if (!this->dialogData)
	{
		IMZADI_LOG_ERROR("Whatever loaded for the dialog data wasn't dialog data.");
		return false;
	}

	Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("Conversation", new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
		this->InitiateConversation(dynamic_cast<const ConversationEvent*>(event));
	}));

	return true;
}

bool DialogSystem::Shutdown()
{
	this->dialogData.Reset();
	this->currentDialogSequence.Reset();
	this->currentDialogSequencePosition = -1;
	return true;
}

bool DialogSystem::PresentlyEngagedInConversation()
{
	return this->currentDialogSequence.Get() != nullptr;
}

bool DialogSystem::InitiateConversation(const ConversationEvent* convoEvent)
{
	if (!convoEvent)
		return false;

	if (this->PresentlyEngagedInConversation())
		return false;

	std::string sequenceName;
	if (!this->DetermineDialogSequenceForConversation(convoEvent, sequenceName))
		return false;

	DialogData::SequenceMap::iterator iter = this->dialogData->sequenceMap.find(sequenceName);
	if (iter == this->dialogData->sequenceMap.end())
		return false;

	this->currentDialogSequence = iter->second;
	this->currentDialogSequencePosition = 0;

	if (this->currentDialogSequence->dialogElementArray.size() == 0)
	{
		this->currentDialogSequence.Reset();
		this->currentDialogSequencePosition = -1;
		return false;
	}

	DialogElement* dialogElement = this->currentDialogSequence->dialogElementArray[this->currentDialogSequencePosition];
	if (!dialogElement->Setup())
	{
		this->currentDialogSequence.Reset();
		this->currentDialogSequencePosition = -1;
		return false;
	}

	Imzadi::Game::Get()->PushControllerUser("DialogSystem");
	return true;
}

bool DialogSystem::DetermineDialogSequenceForConversation(const ConversationEvent* convoEvent, std::string& sequenceName)
{
	sequenceName = "";

	GameApp* game = (GameApp*)Imzadi::Game::Get();
	GameProgress* progress = game->GetGameProgress();

	std::vector<Imzadi::Reference<Imzadi::Entity>> entityArray;

	for (uint32_t handle : convoEvent->participantHandleArray)
	{
		Imzadi::Reference<Imzadi::ReferenceCounted> ref;
		Imzadi::HandleManager::Get()->GetObjectFromHandle(handle, ref);
		Imzadi::Reference<Imzadi::Entity> entity;
		entity.SafeSet(ref.Get());
		if (!entity)
			return false;

		entityArray.push_back(entity);
	}

	if (entityArray.size() == 2)
	{
		std::string otherEntityName;
		if (entityArray[0]->GetName() == "Deanna")
			otherEntityName = entityArray[1]->GetName();
		else if (entityArray[1]->GetName() == "Deanna")
			otherEntityName = entityArray[0]->GetName();
		else
			return false;

		if (otherEntityName == "Lwaxana")
		{
			if (!progress->WasMileStoneReached("initial_contact_with_lwaxana_made"))
				sequenceName = "lwaxana_initial_contact";
			else
				sequenceName = "lwaxana_encourage_deanna";
		}
	}

	return sequenceName.size() > 0;
}

void DialogSystem::Tick()
{
	if (!this->currentDialogSequence)
		return;

	DialogElement* dialogElement = this->currentDialogSequence->dialogElementArray[this->currentDialogSequencePosition];
	std::string nextSequence;
	int nextSequencePosition = this->currentDialogSequencePosition;
	if (dialogElement->Tick(nextSequence, nextSequencePosition))
		return;

	dialogElement->Shutdown();
	dialogElement = nullptr;

	while (true)
	{
		if (nextSequence.length() > 0)
		{
			DialogData::SequenceMap::iterator iter = this->dialogData->sequenceMap.find(nextSequence);
			if (iter == this->dialogData->sequenceMap.end())
				break;

			this->currentDialogSequence = iter->second;
			this->currentDialogSequencePosition = 0;
		}
		else
		{
			if (nextSequencePosition == this->currentDialogSequencePosition)
				break;

			this->currentDialogSequencePosition = nextSequencePosition;
		}

		if (this->currentDialogSequencePosition < 0 || this->currentDialogSequencePosition >= this->currentDialogSequence->dialogElementArray.size())
			break;

		dialogElement = this->currentDialogSequence->dialogElementArray[this->currentDialogSequencePosition];
		if (!dialogElement->Setup())
		{
			dialogElement = nullptr;
			break;
		}

		break;
	}
	
	if (!dialogElement)
	{
		this->currentDialogSequence.Reset();
		this->currentDialogSequencePosition = -1;
		Imzadi::Game::Get()->PopControllerUser();
	}
}

//--------------------------------- ConversationEvent ---------------------------------

ConversationEvent::ConversationEvent()
{
}

/*virtual*/ ConversationEvent::~ConversationEvent()
{
}