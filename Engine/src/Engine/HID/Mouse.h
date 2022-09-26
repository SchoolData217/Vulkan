#pragma once

#include "MouseButton.h"

namespace Engine {

	struct ButtonData
	{
		MouseButton Button = MouseButton::None;
		ButtonState State = ButtonState::None;
		ButtonState OldState = ButtonState::None;
	};

	class Mouse
	{
	public:
		static bool IsPressed(MouseButton button);
		static bool IsTrigger(MouseButton button);
		static bool IsRelease(MouseButton button);

		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();

		static void SetCursorMode(CursorMode mode);
		static CursorMode GetCursorMode();

	private:
		friend class Window;
		static void PollButtonStates();
		static void UpdateButtonState(MouseButton button, ButtonState newState);

	private:
		inline static std::map<MouseButton, ButtonData> s_ButtonData;
	};

}