#pragma once

#define DEBUG_BREAK __debugbreak()

#ifdef DEBUG
#define ENABLE_ASSERTS
#endif

#define ENABLE_VERIFY

#ifdef ENABLE_ASSERTS
#define ENGINE_ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Engine, "Assertion Failed", __VA_ARGS__)
#define ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Client, "Assertion Failed", __VA_ARGS__)

#define ENGINE_ASSERT(condition, ...) { if(!(condition)) { ENGINE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); DEBUG_BREAK; } }
#define ASSERT(condition, ...) { if(!(condition)) { ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); DEBUG_BREAK; } }
#else
#define ENGINE_ASSERT(condition, ...)
#define ASSERT(condition, ...)
#endif

#ifdef ENABLE_VERIFY
#define ENGINE_VERIFY_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Engine, "Verify Failed", __VA_ARGS__)
#define VERIFY_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Client, "Verify Failed", __VA_ARGS__)

#define ENGINE_VERIFY(condition, ...) { if(!(condition)) { ENGINE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); DEBUG_BREAK; } }
#define VERIFY(condition, ...) { if(!(condition)) { VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); DEBUG_BREAK; } }
#else
#define CORE_VERIFY(condition, ...)
#define VERIFY(condition, ...)
#endif
