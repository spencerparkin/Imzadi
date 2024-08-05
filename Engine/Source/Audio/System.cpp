#include "System.h"
#include "Log.h"

using namespace Imzadi;

AudioSystem::AudioSystem()
{
	this->audio = nullptr;
	this->masteringVoice = nullptr;
}

/*virtual*/ AudioSystem::~AudioSystem()
{
}

bool AudioSystem::Initialize()
{
	if (this->audio)
		return false;

	HRESULT result = XAudio2Create(&this->audio, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create XAudio2 interface.  Error code: %x", result);
		return false;
	}

	result = this->audio->CreateMasteringVoice(&this->masteringVoice);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create mastering voice.  Error code: %x", result);
		return false;
	}

	return true;
}

bool AudioSystem::Shutdown()
{
	// TODO: Delete submix voices here before master.

	if (this->masteringVoice)
	{
		this->masteringVoice->DestroyVoice();
		this->masteringVoice = nullptr;
	}

	if (this->audio)
	{
		this->audio->Release();
		this->audio = nullptr;
	}

	return true;
}