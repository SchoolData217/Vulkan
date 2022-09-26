#pragma once

extern Engine::Application* Engine::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
	Engine::InitializeEngine();

	Engine::Application* app = Engine::CreateApplication(argc, argv);
	ENGINE_ASSERT(app, "Client Application is null!");

	app->Run();
	delete app;

	Engine::ShutdownEngine();
}