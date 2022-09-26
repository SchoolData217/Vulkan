#include "epch.h"
#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Engine {

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> engineSinks =
		{
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
		};

		std::vector<spdlog::sink_ptr> clientSinks =
		{
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
		};

		engineSinks[0]->set_pattern("%^[%T] %n: %v%$");
		clientSinks[0]->set_pattern("%^[%T] %n: %v%$");

		s_EngineLogger = std::make_shared<spdlog::logger>("Engine", engineSinks.begin(), engineSinks.end());
		s_EngineLogger->set_level(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("Client", clientSinks.begin(), clientSinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);
	}

	void Log::Shutdown()
	{
		s_ClientLogger.reset();
		s_EngineLogger.reset();
		spdlog::drop_all();
	}

}
