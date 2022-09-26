#pragma once

#include "KeyCode.h"

namespace Engine {

	struct KeyData
	{
		KeyCode Key = KeyCode::None;
		KeyState State = KeyState::None;
		KeyState OldState = KeyState::None;
	};

	class Keyboard
	{
	public:
		static bool IsPressed(KeyCode keycode);
		static bool IsTrigger(KeyCode keycode);
		static bool IsRelease(KeyCode keycode);

	private:
		friend class Window;
		static void PollKeyStates();
		static void UpdateKeyState(KeyCode key, KeyState newState);

	private:
		inline static std::map<KeyCode, KeyData> s_KeyData;
	};

}