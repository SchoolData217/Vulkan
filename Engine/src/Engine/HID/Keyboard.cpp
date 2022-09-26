#include "epch.h"
#include "Keyboard.h"

#include <GLFW/glfw3.h>

#include "Engine/Core/Application.h"

namespace {

	GLFWwindow* GetCurrentWindowHandle()
	{
		return static_cast<GLFWwindow*>(Engine::Application::Get().GetWindow().GetNativeWindow());
	}

}

namespace Engine {

	bool Keyboard::IsPressed(KeyCode keycode)
	{
		auto state = glfwGetKey(GetCurrentWindowHandle(), static_cast<int32_t>(keycode));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Keyboard::IsTrigger(KeyCode keycode)
	{
		return s_KeyData.find(keycode) != s_KeyData.end() && s_KeyData[keycode].State == KeyState::Pressed;
	}

	bool Keyboard::IsRelease(KeyCode keycode)
	{
		return s_KeyData.find(keycode) != s_KeyData.end() && s_KeyData[keycode].State == KeyState::Released;
	}

	void Keyboard::UpdateKeyState(KeyCode key, KeyState newState)
	{
		auto& keyData = s_KeyData[key];
		keyData.Key = key;
		keyData.OldState = keyData.State;
		keyData.State = newState;
	}

	void Keyboard::PollKeyStates()
	{
		// clear released keys
		for (const auto& [key, keyData] : s_KeyData)
		{
			if (keyData.State == KeyState::Released)
				UpdateKeyState(key, KeyState::None);
		}

		// transition pressed keys
		for (const auto& [key, keyData] : s_KeyData)
		{
			if (keyData.State == KeyState::Pressed)
				UpdateKeyState(key, KeyState::Repeat);
		}
	}

}