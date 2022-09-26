#include "epch.h"
#include "Mouse.h"

#include <GLFW/glfw3.h>

#include "Engine/Core/Application.h"

namespace {

	GLFWwindow* GetCurrentWindowHandle()
	{
		return static_cast<GLFWwindow*>(Engine::Application::Get().GetWindow().GetNativeWindow());
	}

}

namespace Engine {

	bool Mouse::IsPressed(MouseButton button)
	{
		auto state = glfwGetMouseButton(GetCurrentWindowHandle(), static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	bool Mouse::IsTrigger(MouseButton button)
	{
		return s_ButtonData.find(button) != s_ButtonData.end() && s_ButtonData[button].State == ButtonState::Pressed;
	}

	bool Mouse::IsRelease(MouseButton button)
	{
		return s_ButtonData.find(button) != s_ButtonData.end() && s_ButtonData[button].State == ButtonState::Released;
	}

	float Mouse::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return (float)x;
	}

	float Mouse::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return (float)y;
	}

	std::pair<float, float> Mouse::GetMousePosition()
	{
		double x = 0, y = 0;
		glfwGetCursorPos(GetCurrentWindowHandle(), &x, &y);
		return { (float)x, (float)y };
	}

	void Mouse::SetCursorMode(CursorMode mode)
	{
		glfwSetInputMode(GetCurrentWindowHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}

	CursorMode Mouse::GetCursorMode()
	{
		return static_cast<CursorMode>((glfwGetInputMode(GetCurrentWindowHandle(), GLFW_CURSOR) - GLFW_CURSOR_NORMAL));
	}

	void Mouse::UpdateButtonState(MouseButton button, ButtonState newState)
	{
		auto& buttonData = s_ButtonData[button];
		buttonData.Button = button;
		buttonData.OldState = buttonData.State;
		buttonData.State = newState;
	}

	void Mouse::PollButtonStates()
	{
		// clear released buttons
		for (const auto& [button, buttonData] : s_ButtonData)
		{
			if (buttonData.State == ButtonState::Released)
				UpdateButtonState(button, ButtonState::None);
		}

		// transition pressed buttons
		for (const auto& [button, buttonData] : s_ButtonData)
		{
			if (buttonData.State == ButtonState::Pressed)
				UpdateButtonState(button, ButtonState::Hold);
		}
	}

}