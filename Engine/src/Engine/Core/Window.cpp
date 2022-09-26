#include "epch.h"
#include "Window.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "Engine/Core/Application.h"
#include "Engine/Core/Events/KeyEvent.h"
#include "Engine/Core/Events/MouseEvent.h"
#include "Engine/HID/Keyboard.h"
#include "Engine/HID/Mouse.h"

namespace {

	bool s_GLFWInitialized = false;

	void GLFWErrorCallback(int error, const char* description)
	{
		LOG_ENGINE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

}

namespace Engine {

	Window::Window(const ApplicationConfig& config)
	{
		m_Data.Title = config.Name;
		m_Data.Width = config.WindowWidth;
		m_Data.Height = config.WindowHeight;

		// GLFW Init
		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			ENGINE_ASSERT(success, "Could not intialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		// Create Window
		if (!config.WindowDecorated)
		{
			glfwWindowHint(GLFW_DECORATED, false);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, (int)config.Resizable);

		if (config.Fullscreen)
		{
			GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

			glfwWindowHint(GLFW_DECORATED, false);
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

			m_Window = glfwCreateWindow(mode->width, mode->height, m_Data.Title.c_str(), primaryMonitor, nullptr);
		}
		else
		{
			m_Window = glfwCreateWindow(config.WindowWidth, config.WindowHeight, m_Data.Title.c_str(), nullptr, nullptr);
		}
		ENGINE_ASSERT(m_Window, "Could not create Window!");

		glfwSetWindowUserPointer(m_Window, &m_Data);

		// Check Raw MouseMotion Supported
		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		else
			LOG_ENGINE_WARN_TAG("Platform", "Raw mouse motion not supported.");

		SetGLFWCallbacks();

		// Update window size to actual size
		{
			int width, height;
			glfwGetWindowSize(m_Window, &width, &height);
			m_Data.Width = width;
			m_Data.Height = height;
		}

		// Set icon
		{
			GLFWimage icon;
			int channels;
			icon.pixels = stbi_load("Resources/Editor/Logo.png", &icon.width, &icon.height, &channels, 4);
			glfwSetWindowIcon(m_Window, 1, &icon);
			stbi_image_free(icon.pixels);
		}
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Init()
	{

	}

	void Window::ProcessEvents()
	{
		Keyboard::PollKeyStates();
		Mouse::PollButtonStates();
		glfwPollEvents();
	}

	void Window::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	void Window::Maximize() const
	{
		glfwMaximizeWindow(m_Window);
	}

	void Window::Restore() const
	{
		glfwRestoreWindow(m_Window);
	}

	void Window::Minimize() const
	{
		glfwIconifyWindow(m_Window);
	}

	void Window::CenterWindow() const
	{
		const GLFWvidmode* videmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		int x = (videmode->width / 2) - (m_Data.Width / 2);
		int y = (videmode->height / 2) - (m_Data.Height / 2);
		glfwSetWindowPos(m_Window, x, y);
	}

	bool Window::IsMaximized() const
	{
		return (bool)glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED);
	}

	void Window::SetGLFWCallbacks()
	{
		// window close
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				WindowCloseEvent event;
				data.EventCallback(event);
			});

		// window minimize
		glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));
				WindowMinimizeEvent event((bool)iconified);
				data.EventCallback(event);
			});

		// window resize
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				WindowResizeEvent event((uint32_t)width, (uint32_t)height);
				data.EventCallback(event);
				data.Width = width;
				data.Height = height;
			});

		// keyboard
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					Keyboard::UpdateKeyState((KeyCode)key, KeyState::Pressed);
					KeyPressedEvent event((KeyCode)key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					Keyboard::UpdateKeyState((KeyCode)key, KeyState::Released);
					KeyReleasedEvent event((KeyCode)key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					Keyboard::UpdateKeyState((KeyCode)key, KeyState::Repeat);
					KeyPressedEvent event((KeyCode)key, 1);
					data.EventCallback(event);
					break;
				}
				}
			});

		// mouse scroll
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});

		// mouse button
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					Mouse::UpdateButtonState((MouseButton)button, ButtonState::Pressed);
					MouseButtonPressedEvent event((MouseButton)button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					Mouse::UpdateButtonState((MouseButton)button, ButtonState::Released);
					MouseButtonReleasedEvent event((MouseButton)button);
					data.EventCallback(event);
					break;
				}
				}
			});
	}

	void Window::Shutdown()
	{
		glfwTerminate();
		s_GLFWInitialized = false;
	}

}