#pragma once

#include "Reference.h"
#include "EventSystem.h"

class DialogData;
class DialogSequence;
class ConversationEvent;

/**
 * One of the basic functions of this system is simply to facilitate the
 * execution of a dialog sequence.  The most challenging part of a dialog
 * system, however, is simply that of knowing which dialog sequence
 * should be invoked at any particular time.  How much this system will
 * aid in that responsability, shared with the game, is not entirely clear
 * to me at the time of this writing.  Context and continuity and persistence
 * of memory are all factors here, and it doesn't seem trivial to me.
 */
class DialogSystem
{
public:
	DialogSystem();
	virtual ~DialogSystem();

	bool Initialize();
	bool Shutdown();
	void Tick();
	bool InitiateConversation(const ConversationEvent* convoEvent);
	bool PresentlyEngagedInConversation();

	const std::vector<uint32_t>& GetCurrentParticipantArray() const { return this->currentParticipantHandleArray; }

private:
	bool DetermineDialogSequenceForConversation(const ConversationEvent* convoEvent, std::string& sequenceName);

	Imzadi::Reference<DialogData> dialogData;
	Imzadi::Reference<DialogSequence> currentDialogSequence;
	int currentDialogSequencePosition;
	std::vector<uint32_t> currentParticipantHandleArray;
};

/**
 * These types of events are sent to initiate conversations in the dialog system.
 */
class ConversationEvent : public Imzadi::Event
{
public:
	ConversationEvent();
	virtual ~ConversationEvent();

	virtual Event* New() const override;
	virtual Event* Clone() const override;

	bool IsParticipant(uint32_t handle) const;

public:
	std::vector<uint32_t> participantHandleArray;
};

/**
 * These types of events are sent at the beginning and ending of a conversation.
 */
class ConvoBoundaryEvent : public ConversationEvent
{
public:
	enum Type
	{
		UNKNOWN,
		STARTED,
		FINISHED
	};

	ConvoBoundaryEvent();
	ConvoBoundaryEvent(Type type, DialogSystem* dialogSystem);
	virtual ~ConvoBoundaryEvent();

	virtual Event* New() const override;
	virtual Event* Clone() const override;

public:
	Type type;
};