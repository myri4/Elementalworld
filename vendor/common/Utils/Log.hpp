#pragma once

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace wc {

	class Log
	{
	public:
		static void Init()
		{
			std::array<spdlog::sink_ptr, 2> logSinks;
			logSinks[0] = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			logSinks[1] = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log/APP.log", true);

			logSinks[0]->set_pattern("%^[%D %T][%l] %n: %v%$");
			logSinks[1]->set_pattern("%^[%D %T][%l] %n: %v%$");

			GetLogger() = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
			spdlog::register_logger(Log::GetLogger());
			GetLogger()->set_level(spdlog::level::trace);
			GetLogger()->flush_on(spdlog::level::trace);
		}

		static std::shared_ptr<spdlog::logger>& GetLogger() { 
			static std::shared_ptr<spdlog::logger> s_Logger;
			return s_Logger; 
		}
	};

}
// Client log macros
#define WC_TRACE(...)         wc::Log::GetLogger()->trace(__VA_ARGS__)
#define WC_INFO(...)          wc::Log::GetLogger()->info(__VA_ARGS__)
#define WC_WARN(...)          wc::Log::GetLogger()->warn(__VA_ARGS__)
#define WC_ERROR(...)         wc::Log::GetLogger()->error(__VA_ARGS__)
#define WC_CRITICAL(...)      wc::Log::GetLogger()->critical(__VA_ARGS__)