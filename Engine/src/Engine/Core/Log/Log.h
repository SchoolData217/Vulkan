#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <map>

#include "LogCustomFormatters.h"

namespace Engine {

	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Engine = 0, Client = 1
		};
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};
		struct TagDetails
		{
			bool Enabled = true;
			Level LevelFilter = Level::Trace;
		};

	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		static bool HasTag(const std::string& tag) { return s_EnabledTags.find(tag) != s_EnabledTags.end(); }
		static std::map<std::string, TagDetails>& EnabledTags() { return s_EnabledTags; }

		template<typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args);

		template<typename... Args>
		static void PrintAssertMessage(Log::Type type, std::string_view prefix, Args&&... args);

	public:
		// Enum utils
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
			case Level::Trace: return "Trace";
			case Level::Info:  return "Info";
			case Level::Warn:  return "Warn";
			case Level::Error: return "Error";
			case Level::Fatal: return "Fatal";
			}
			return "";
		}
		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info")  return Level::Info;
			if (string == "Warn")  return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;

			return Level::Trace;
		}

	private:
		inline static std::shared_ptr<spdlog::logger> s_EngineLogger;
		inline static std::shared_ptr<spdlog::logger> s_ClientLogger;

		inline static std::map<std::string, TagDetails> s_EnabledTags;
	};

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tagged logs (prefer these!)                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Engine logging
#define LOG_ENGINE_TRACE_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Trace,	tag, __VA_ARGS__)
#define LOG_ENGINE_INFO_TAG(tag, ...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Info,	tag, __VA_ARGS__)
#define LOG_ENGINE_WARN_TAG(tag, ...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Warn,	tag, __VA_ARGS__)
#define LOG_ENGINE_ERROR_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Error,	tag, __VA_ARGS__)
#define LOG_ENGINE_FATAL_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Fatal,	tag, __VA_ARGS__)

// Client logging
#define LOG_TRACE_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Trace,	tag, __VA_ARGS__)
#define LOG_INFO_TAG(tag, ...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Info,		tag, __VA_ARGS__)
#define LOG_WARN_TAG(tag, ...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Warn,		tag, __VA_ARGS__)
#define LOG_ERROR_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Error,	tag, __VA_ARGS__)
#define LOG_FATAL_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Fatal,	tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Engine Logging
#define LOG_ENGINE_TRACE(...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Trace, "", __VA_ARGS__)
#define LOG_ENGINE_INFO(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Info, "", __VA_ARGS__)
#define LOG_ENGINE_WARN(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Warn, "", __VA_ARGS__)
#define LOG_ENGINE_ERROR(...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Error, "", __VA_ARGS__)
#define LOG_ENGINE_FATAL(...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Engine, ::Engine::Log::Level::Fatal, "", __VA_ARGS__)

// Client Logging
#define LOG_TRACE(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Trace, "", __VA_ARGS__)
#define LOG_INFO(...)    ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Info, "", __VA_ARGS__)
#define LOG_WARN(...)    ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Warn, "", __VA_ARGS__)
#define LOG_ERROR(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Error, "", __VA_ARGS__)
#define LOG_FATAL(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Fatal, "", __VA_ARGS__)

namespace Engine {

	template<typename... Args>
	void Log::PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args)
	{
		auto detail = s_EnabledTags[std::string(tag)];
		if (detail.Enabled && detail.LevelFilter <= level)
		{
			auto logger = (type == Type::Engine) ? GetEngineLogger() : GetClientLogger();
			std::string logString = tag.empty() ? "{0}{1}" : "[{0}] {1}";
			switch (level)
			{
			case Level::Trace:
				logger->trace(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Info:
				logger->info(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Warn:
				logger->warn(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Error:
				logger->error(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Fatal:
				logger->critical(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			}
		}
	}

	template<typename... Args>
	void Log::PrintAssertMessage(Log::Type type, std::string_view prefix, Args&&... args)
	{
		auto logger = (type == Type::Engine) ? GetEngineLogger() : GetClientLogger();
		logger->error("{0}: {1}", prefix, fmt::format(std::forward<Args>(args)...));
	}

	template<>
	inline void Log::PrintAssertMessage(Log::Type type, std::string_view prefix)
	{
		auto logger = (type == Type::Engine) ? GetEngineLogger() : GetClientLogger();
		logger->error("{0}", prefix);
	}
}