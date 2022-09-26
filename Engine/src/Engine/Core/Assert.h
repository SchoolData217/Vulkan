#pragma once

#ifdef HZ_PLATFORM_WINDOWS
#define HZ_DEBUG_BREAK __debugbreak()
#else
#define HZ_DEBUG_BREAK
#endif

#ifdef DEBUG
#define HZ_ENABLE_ASSERTS
#endif

#define HZ_ENABLE_VERIFY

#ifdef HZ_ENABLE_ASSERTS
#define HZ_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Engine, "Assertion Failed", __VA_ARGS__)
#define HZ_ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Client, "Assertion Failed", __VA_ARGS__)

#define HZ_CORE_ASSERT(condition, ...) { if(!(condition)) { HZ_CORE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); HZ_DEBUG_BREAK; } }
#define HZ_ASSERT(condition, ...) { if(!(condition)) { HZ_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); HZ_DEBUG_BREAK; } }
#else
#define HZ_CORE_ASSERT(condition, ...)
#define HZ_ASSERT(condition, ...)
#endif

#ifdef HZ_ENABLE_VERIFY
#define HZ_CORE_VERIFY_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Engine, "Verify Failed", __VA_ARGS__)
#define HZ_VERIFY_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Client, "Verify Failed", __VA_ARGS__)

#define HZ_CORE_VERIFY(condition, ...) { if(!(condition)) { HZ_CORE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); HZ_DEBUG_BREAK; } }
#define HZ_VERIFY(condition, ...) { if(!(condition)) { HZ_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); HZ_DEBUG_BREAK; } }
#else
#define HZ_CORE_VERIFY(condition, ...)
#define HZ_VERIFY(condition, ...)
#endif
