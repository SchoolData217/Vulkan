#pragma once

#include "Engine/Config/ApplicationConfig.h"

#include "Events/Event.h"

struct GLFWwindow;

namespace Engine {

	class Window
	{
		using EventCallbackFn = std::function<void(Event&)>;

	public:
		Window(const ApplicationConfig& config);
		~Window();

		void Init();

		void ProcessEvents();

		inline void* GetNativeWindow() const { return m_Window; }
		inline uint32_t GetWidth() const { return m_Data.Width; }
		inline uint32_t GetHeight() const { return m_Data.Height; }

		void SetVSync(bool enabled);
		void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

		void Maximize() const;
		void Restore() const;
		void Minimize() const;
		void CenterWindow() const;

		bool IsMaximized() const;

	private:
		void SetGLFWCallbacks();
		void Shutdown();

	private:
		GLFWwindow* m_Window = nullptr;

		struct WindowData
		{
			std::string Title = "";
			uint32_t Width = 0, Height = 0;
			bool VSync = true;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}