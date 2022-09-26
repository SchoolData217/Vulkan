#pragma once

#include <queue>

#include "Engine/Config/ApplicationConfig.h"

#include "Timestep.h"
#include "Timer.h"
#include "Window.h"

#include "Engine/Core/Layers/LayerStack.h"

#include "Events/ApplicationEvent.h"

namespace Engine {

	class ImGuiLayer;

	class Application
	{
		using EventCallbackFn = std::function<void(Event&)>;

	public:
		Application(const ApplicationConfig& config);
		virtual ~Application();

		void Run();
		void Close();

		virtual void OnInit() {}
		virtual void OnShutdown();
		virtual void OnUpdate(Timestep ts) {}

		virtual void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);
		void RenderImGui();

		inline Window& GetWindow() { return *m_Window; }

		static inline Application& Get() { return *s_Instance; }

		Timestep GetTimestep() const { return m_TimeStep; }
		Timestep GetFrametime() const { return m_Frametime; }
		float GetTime() const;

		template<typename TEvent, bool DispatchImmediately = false, typename... TEventArgs>
		void DispatchEvent(TEventArgs&&... args);

		template<typename Func>
		void QueueEvent(Func&& func);

	private:
		void ProcessEvents();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowMinimize(WindowMinimizeEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationConfig m_Config;
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		Timestep m_Frametime;
		Timestep m_TimeStep;

		std::mutex m_EventQueueMutex;
		std::queue<std::function<void()>> m_EventQueue;

		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
	};

	template<typename TEvent, bool DispatchImmediately, typename... TEventArgs>
	inline void Application::DispatchEvent(TEventArgs && ...args)
	{
		static_assert(std::is_assignable_v<Event, TEvent>);

		std::shared_ptr<TEvent> event = std::make_shared<TEvent>(std::forward<TEventArgs>(args)...);
		if constexpr (DispatchImmediately)
		{
			OnEvent(*event);
		}
		else
		{
			std::scoped_lock<std::mutex> lock(m_EventQueueMutex);
			m_EventQueue.push([event]() { Application::Get().OnEvent(*event); });
		}
	}

	template<typename Func>
	inline void Application::QueueEvent(Func&& func)
	{
		m_EventQueue.push(func);
	}


	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);

}
