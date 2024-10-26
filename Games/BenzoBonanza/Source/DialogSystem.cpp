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

	Imzadi::Game::Get()->GetEventSystem()->RegisterEventListener("Conversation", Imzadi::EventListenerType::PERMINANT, new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
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

	this->currentParticipantHandleArray = convoEvent->participantHandleArray;
	Imzadi::Game::Get()->GetEventSystem()->SendEvent("ConvoBoundary", new ConvoBoundaryEvent(ConvoBoundaryEvent::Type::STARTED, this));
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
		if (entityArray[0]->GetName() == "Alice")
			otherEntityName = entityArray[1]->GetName();
		else if (entityArray[1]->GetName() == "Alice")
			otherEntityName = entityArray[0]->GetName();
		else
			return false;

		if (otherEntityName == "Spencer")
		{
			if (!progress->WasMileStoneReached("initial_contact_with_spencer_made"))
				sequenceName = "spencer_initial_contact";
			else
			{
				int totalNumBenzos = 0, numBenzosReturned = 0;
				progress->CalcBenzoStats(totalNumBenzos, numBenzosReturned);
				if (totalNumBenzos == numBenzosReturned)
				{
					if (!progress->WasMileStoneReached("celebration_discussion_had"))
						sequenceName = "you_won_the_game";
					else
						sequenceName = "final_spencer_talk";
				}
				else if (progress->GetPossessedBenzoCount() == 0)
					sequenceName = "no_benzos_to_give_convo";
				else
					sequenceName = "benzos_to_give_convo";
			}
		}
		else if (otherEntityName == "Bob")
		{
			static int convoNumber = 0;
			convoNumber++;
			if (convoNumber > 20)
				convoNumber = 1;

			sequenceName = std::format("bob_convo_{}", convoNumber);
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
		Imzadi::Game::Get()->GetEventSystem()->SendEvent("ConvoBoundary", new ConvoBoundaryEvent(ConvoBoundaryEvent::Type::FINISHED, this));
		this->currentParticipantHandleArray.clear();
	}
}

//--------------------------------- ConversationEvent ---------------------------------

ConversationEvent::ConversationEvent()
{
}

/*virtual*/ ConversationEvent::~ConversationEvent()
{
}

/*virtual*/ Imzadi::Event* ConversationEvent::New() const
{
	return new ConversationEvent();
}

/*virtual*/ Imzadi::Event* ConversationEvent::Clone() const
{
	auto event = (ConversationEvent*)Event::Clone();
	for (uint32_t handle : this->participantHandleArray)
		event->participantHandleArray.push_back(handle);
	return event;
}

bool ConversationEvent::IsParticipant(uint32_t handle) const
{
	for (int i = 0; i < (signed)this->participantHandleArray.size(); i++)
		if (handle == this->participantHandleArray[i])
			return true;

	return false;
}

//--------------------------------- ConvoBoundaryEvent ---------------------------------

ConvoBoundaryEvent::ConvoBoundaryEvent()
{
	this->type = Type::UNKNOWN;
}

ConvoBoundaryEvent::ConvoBoundaryEvent(Type type, DialogSystem* dialogSystem)
{
	this->type = type;
	this->participantHandleArray = dialogSystem->GetCurrentParticipantArray();
}

/*virtual*/ ConvoBoundaryEvent::~ConvoBoundaryEvent()
{
}

/*virtual*/ Imzadi::Event* ConvoBoundaryEvent::New() const
{
	return new ConvoBoundaryEvent();
}

/*virtual*/ Imzadi::Event* ConvoBoundaryEvent::Clone() const
{
	auto event = (ConvoBoundaryEvent*)ConversationEvent::Clone();
	event->type = this->type;
	return event;
}