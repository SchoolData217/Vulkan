#include "epch.h"
#include "Application.h"

#include <GLFW/glfw3.h>

#include "Engine/Renderer/Vulkan/VulkanTest.h"

namespace {

	Engine::VulkanTest s_VulkanTest;

}

namespace Engine {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationConfig& config)
		: m_Config(config)
	{
		s_Instance = this;

		// create windows window
		m_Window = std::make_unique<Window>(m_Config);
		m_Window->Init();
		m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

		//m_Window->SetVSync(m_Config.VSync);
		if (m_Config.Maximized)
			m_Window->Maximize();
		else
			m_Window->CenterWindow();
#if 0
		if (m_Config.EnableImGui)
		{
			m_ImGuiLayer = ImGuiLayer::Create();
			PushOverlay(m_ImGuiLayer);
		}
#endif
#ifdef DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif



		s_VulkanTest.Initialize((GLFWwindow*)m_Window->GetNativeWindow());
	}

	Application::~Application()
	{
		m_Window->SetEventCallback([](Event& e) {});

		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
		layer->OnDetach();
	}

	void Application::PopOverlay(Layer* layer)
	{
		m_LayerStack.PopOverlay(layer);
		layer->OnDetach();
	}

	void Application::RenderImGui()
	{
#if 0
		m_ImGuiLayer->Begin();

		for (int i = 0; i < m_LayerStack.Size(); i++)
			m_LayerStack[i]->OnImGuiRender();
#endif
	}

	void Application::Run()
	{
		OnInit();

		while (m_Running)
		{
			ProcessEvents();

			if (!m_Minimized)
			{
				{
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(m_TimeStep);
#if 0
					if (m_Config.EnableImGui)
					{
						RenderImGui();
						m_ImGuiLayer->End();
					}
#endif
		}

				s_VulkanTest.Render();
	}

			float time = GetTime();
			m_Frametime = time - m_LastFrameTime;
			m_TimeStep = glm::min<float>(m_Frametime, 0.0333f);
			m_LastFrameTime = time;
}

		s_VulkanTest.Terminate();
		OnShutdown();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnShutdown()
	{

	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });
		dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return OnWindowMinimize(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}

		if (event.Handled)
			return;
	}

	float Application::GetTime() const
	{
		return (float)glfwGetTime();
	}

	void Application::ProcessEvents()
	{
		m_Window->ProcessEvents();

		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		// Process custom event queue
		while (m_EventQueue.size() > 0)
		{
			auto& func = m_EventQueue.front();
			func();
			m_EventQueue.pop();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();
		return false; // give other things a chance to react to window close
	}

	bool Application::OnWindowMinimize(WindowMinimizeEvent& e)
	{
		m_Minimized = e.IsMinimized();
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		const uint32_t width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			return false;
		}

		//m_Window->GetSwapChain().OnResize(width, height);

		return false;
	}

}
