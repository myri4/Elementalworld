#pragma once
#include <wclibs/Core.hpp>
// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)


namespace wc {

	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetLogger() { return s_Logger; }
	private:
		static Ref<spdlog::logger> s_Logger;
	};

}
// Client log macros
#define WC_TRACE(...)         ::wc::Log::GetLogger()->trace(__VA_ARGS__)
#define WC_INFO(...)          ::wc::Log::GetLogger()->info(__VA_ARGS__)
#define WC_WARN(...)          ::wc::Log::GetLogger()->warn(__VA_ARGS__)
#define WC_ERROR(...)         ::wc::Log::GetLogger()->error(__VA_ARGS__)
#define WC_CRITICAL(...)      ::wc::Log::GetLogger()->critical(__VA_ARGS__)