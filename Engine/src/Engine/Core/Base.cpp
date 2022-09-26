#include "epch.h"
#include "Base.h"

void Engine::InitializeEngine()
{
	Log::Init();

	LOG_ENGINE_INFO("Engine Initializing...");
}

void Engine::ShutdownEngine()
{
	LOG_ENGINE_INFO("Engine Shutting Down...");

	Log::Shutdown();
}
