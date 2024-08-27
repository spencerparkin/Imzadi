#pragma once

#include "Input.h"
#include <unordered_map>

namespace Imzadi
{
	/**
	 * Provide input using keyboard/mouse.
	 */
	class IMZADI_API PCInput : public Input
	{
	public:
		PCInput(int playerNumber);
		virtual ~PCInput();

		virtual bool Setup(HWND windowHandle) override;
		virtual bool Shutdown() override;
		virtual void Update(double deltaTime) override;
		virtual void HandleInputMessage(WPARAM wparam, LPARAM lParam) override;
		virtual Vector2 GetAnalogJoyStick(Button button) override;
		virtual double GetTrigger(Button button) override;
		virtual bool ButtonPressed(Button button, bool consume = false) override;
		virtual bool ButtonReleased(Button button, bool consume = false) override;
		virtual bool ButtonDown(Button button) override;
		virtual bool ButtonUp(Button button) override;

	private:

		void HandleKeyboardMessage(const RAWKEYBOARD& rawKeyboard);
		void HandleMouseMessage(const RAWMOUSE& rawMouse);

		enum KeyState
		{
			UP,
			DOWN
		};

		enum KeyAction
		{
			NONE,
			PRESSED,
			RELEASED
		};

		struct KeyInfo
		{
			KeyState state;
			KeyAction action;
		};

		bool GetKeyInfo(USHORT key, KeyInfo& info) const;
		bool SetKeyInfo(USHORT key, const KeyInfo& info);

		USHORT ButtonToKey(Button button);

		bool IsKeyDown(USHORT key);
		bool IsKeyUp(USHORT key);

		typedef std::unordered_map<USHORT, KeyInfo> KeyInfoMap;
		KeyInfoMap keyInfoMap;
	};
}