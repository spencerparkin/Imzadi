#include "DialogSystem.h"
#include "Assets/DialogData.h"
#include "CustomAssetCache.h"
#include "GameApp.h"
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

	// This is where we know what people all want to talk to one another, and where
	// we then decide, based on that and the progress of the game, what dialog sequence
	// they are going to play out.  The details of how to do this are still a bit in
	// the air, so just do something simple for now.

	// TODO: Right now, this is the only authored dialog sequence, so just do this one.  But of course, this will need to change.
	DialogData::SequenceMap::iterator iter = this->dialogData->sequenceMap.find("lwaxana_initial_contact");
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