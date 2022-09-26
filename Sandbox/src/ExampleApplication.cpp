#include <Engine.h>
#include <Engine/EntryPoint.h>

using namespace Engine;

class ExampleLayer : public Engine::Layer
{
public:
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(Event& event) override;
	virtual void OnUpdate(Engine::Timestep ts) override;
	virtual void OnImGuiRender() override;

private:
	bool OnKeyPressed(KeyPressedEvent& event);
};

class ExampleApplication : public Engine::Application
{
public:
	ExampleApplication(const Engine::ApplicationConfig& config)
		: Application(config)
	{
		PushLayer(new ExampleLayer());
	}
};

Engine::Application* Engine::CreateApplication(int argc, char** argv)
{
	Engine::ApplicationConfig config;
	config.Name = "ExampleApplication";
	//config.VSync = false;

	config.WindowWidth = 1600;
	config.WindowHeight = 900;

	return new ExampleApplication(config);
}

void ExampleLayer::OnAttach()
{

}

void ExampleLayer::OnDetach()
{
}

void ExampleLayer::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) {return OnKeyPressed(e); });
}

bool ExampleLayer::OnKeyPressed(KeyPressedEvent& event)
{
	if (event.GetKeyCode() == KeyCode::Escape)
		Application::Get().Close();

	return false;
}

void ExampleLayer::OnUpdate(Engine::Timestep ts)
{

}

void ExampleLayer::OnImGuiRender()
{

}