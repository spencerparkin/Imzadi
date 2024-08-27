#include "PCInput.h"
#include "Log.h"

using namespace Imzadi;

PCInput::PCInput(int playerNumber) : Input(playerNumber)
{
}

/*virtual*/ PCInput::~PCInput()
{
}

/*virtual*/ bool PCInput::Setup(HWND windowHandle)
{
	if (this->playerNumber != 0)
		return false;

	UINT numDevices = 0;
	if (GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST)) == -1)
		return false;

	std::unique_ptr<RAWINPUTDEVICELIST> deviceListArray(new RAWINPUTDEVICELIST[numDevices]);
	if (GetRawInputDeviceList(deviceListArray.get(), &numDevices, sizeof(RAWINPUTDEVICELIST)) == -1)
		return false;

	bool hasKeyboard = false;
	bool hasMouse = false;
	for (UINT i = 0; i < numDevices; i++)
	{
		RAWINPUTDEVICELIST* entry = &deviceListArray.get()[i];
		if (entry->dwType == RIM_TYPEKEYBOARD)
			hasKeyboard = true;
		else if (entry->dwType == RIM_TYPEMOUSE)
			hasMouse = true;
	}

	if (!(hasKeyboard && hasMouse))
		return false;

	RAWINPUTDEVICE rawInputDeviceArray[2];

	rawInputDeviceArray[0].usUsagePage = 0x01;	// Usage generic.
	rawInputDeviceArray[0].usUsage = 0x02;		// Mouse.
	rawInputDeviceArray[0].dwFlags = 0;
	rawInputDeviceArray[0].hwndTarget = windowHandle;

	rawInputDeviceArray[1].usUsagePage = 0x01;	// Usage generic.
	rawInputDeviceArray[1].usUsage = 0x06;		// Keyboard.
	rawInputDeviceArray[1].dwFlags = 0;
	rawInputDeviceArray[1].hwndTarget = windowHandle;

	if (RegisterRawInputDevices(rawInputDeviceArray, _countof(rawInputDeviceArray), sizeof(RAWINPUTDEVICE)) == FALSE)
	{
		DWORD error = GetLastError();
		IMZADI_LOG_ERROR("Failed to register to receive raw input from keyboard and mouse with error code: %d", error);
		return false;
	}

	return true;
}

/*virtual*/ bool PCInput::Shutdown()
{
	this->keyInfoMap.clear();
	return true;
}

/*virtual*/ void PCInput::Update(double deltaTime)
{
	std::vector<USHORT> keyArray;
	for (auto pair : this->keyInfoMap)
		keyArray.push_back(pair.first);

	for (USHORT key : keyArray)
	{
		KeyInfo info;
		this->GetKeyInfo(key, info);
		info.action = KeyAction::NONE;
		this->SetKeyInfo(key, info);
	}
}

/*virtual*/ void PCInput::HandleInputMessage(WPARAM wparam, LPARAM lParam)
{
	auto rawInputHandle = reinterpret_cast<HRAWINPUT>(lParam);

	UINT size = 0;
	if (GetRawInputData(rawInputHandle, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER)) == -1)
		return;

	std::unique_ptr<BYTE> rawInputBuffer(new BYTE[size]);
	if (GetRawInputData(rawInputHandle, RID_INPUT, rawInputBuffer.get(), &size, sizeof(RAWINPUTHEADER)) == -1)
		return;

	auto rawInput = reinterpret_cast<RAWINPUT*>(rawInputBuffer.get());
	switch (rawInput->header.dwType)
	{
		case RIM_TYPEKEYBOARD:
		{
			this->HandleKeyboardMessage(rawInput->data.keyboard);
			break;
		}
		case RIM_TYPEMOUSE:
		{
			this->HandleMouseMessage(rawInput->data.mouse);
			break;
		}
	}
}

void PCInput::HandleKeyboardMessage(const RAWKEYBOARD& rawKeyboard)
{
	USHORT key = rawKeyboard.VKey;

	KeyInfo info;
	if (!this->GetKeyInfo(key, info))
		return;

	KeyState oldState = info.state;
	KeyState newState = ((rawKeyboard.Flags & RI_KEY_BREAK) != 0) ? KeyState::UP : KeyState::DOWN;

	if (newState != oldState)
	{
		info.state = newState;

		switch (newState)
		{
		case KeyState::DOWN:
			info.action = KeyAction::PRESSED;
			break;
		case KeyState::UP:
			info.action = KeyAction::RELEASED;
			break;
		}

		this->SetKeyInfo(key, info);
	}
}

void PCInput::HandleMouseMessage(const RAWMOUSE& rawMouse)
{
}

bool PCInput::GetKeyInfo(USHORT key, KeyInfo& info) const
{
	KeyInfoMap::const_iterator iter = this->keyInfoMap.find(key);
	if (iter != this->keyInfoMap.end())
		info = iter->second;
	else
	{
		info.state = KeyState::UP;
		info.action = KeyAction::NONE;
	}

	return true;
}

bool PCInput::SetKeyInfo(USHORT key, const KeyInfo& info)
{
	KeyInfoMap::iterator iter = this->keyInfoMap.find(key);
	if (iter != this->keyInfoMap.end())
		this->keyInfoMap.erase(iter);

	this->keyInfoMap.insert(std::pair<USHORT, KeyInfo>(key, info));
	return true;
}

/*virtual*/ Vector2 PCInput::GetAnalogJoyStick(Button button)
{
	Vector2 vector(0.0, 0.0);

	switch (button)
	{
		case Button::L_JOY_STICK:
		{
			if (this->IsKeyDown('A'))
				vector.x -= 1.0;
			if (this->IsKeyDown('D'))
				vector.x += 1.0;
			if (this->IsKeyDown('S'))
				vector.y -= 1.0;
			if (this->IsKeyDown('W'))
				vector.y += 1.0;
			break;
		}
		case Button::R_JOY_STICK:
		{
			if (this->IsKeyDown(VK_LEFT))
				vector.x -= 1.0;
			if (this->IsKeyDown(VK_RIGHT))
				vector.x += 1.0;
			if (this->IsKeyDown(VK_DOWN))
				vector.y -= 1.0;
			if (this->IsKeyDown(VK_UP))
				vector.y += 1.0;
			break;
		}
	}

	return vector;
}

/*virtual*/ double PCInput::GetTrigger(Button button)
{
	return 0.0;
}

/*virtual*/ bool PCInput::ButtonPressed(Button button, bool consume /*= false*/)
{
	USHORT key = this->ButtonToKey(button);
	if (key == 0)
		return false;

	KeyInfo info;
	if (!this->GetKeyInfo(key, info))
		return false;

	bool buttonPressed(info.action == KeyAction::PRESSED);

	if (consume)
	{
		info.action = KeyAction::NONE;
		this->SetKeyInfo(key, info);
	}

	return buttonPressed;
}

/*virtual*/ bool PCInput::ButtonReleased(Button button, bool consume /*= false*/)
{
	USHORT key = this->ButtonToKey(button);
	if (key == 0)
		return false;

	KeyInfo info;
	if (!this->GetKeyInfo(key, info))
		return false;

	bool buttonReleased(info.action == KeyAction::RELEASED);

	if (consume)
	{
		info.action = KeyAction::NONE;
		this->SetKeyInfo(key, info);
	}

	return buttonReleased;
}

/*virtual*/ bool PCInput::ButtonDown(Button button)
{
	USHORT key = this->ButtonToKey(button);
	if (key == 0)
		return false;

	return this->IsKeyDown(key);
}

/*virtual*/ bool PCInput::ButtonUp(Button button)
{
	USHORT key = this->ButtonToKey(button);
	if (key == 0)
		return false;

	return this->IsKeyUp(key);
}

bool PCInput::IsKeyDown(USHORT key)
{
	KeyInfo info;
	if (!this->GetKeyInfo(key, info))
		return false;

	return info.state == KeyState::DOWN;
}

bool PCInput::IsKeyUp(USHORT key)
{
	KeyInfo info;
	if (!this->GetKeyInfo(key, info))
		return false;

	return info.state == KeyState::UP;
}

USHORT PCInput::ButtonToKey(Button button)
{
	switch (button)
	{
	case Button::X_BUTTON:
		return 'X';
	case Button::Y_BUTTON:
		return VK_SPACE;
	case Button::A_BUTTON:
		return 'A';
	case Button::B_BUTTON:
		return 'B';
	}

	return 0;
}