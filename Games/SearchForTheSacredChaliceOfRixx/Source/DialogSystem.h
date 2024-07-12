#pragma once

#include "Reference.h"

class DialogData;

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

private:
	Imzadi::Reference<DialogData> dialogData;
};