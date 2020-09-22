#pragma once
#include "../wclibspch.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
namespace wc{
    class Log
    {
    private:
        static Ref<spdlog::logger>& CoreLogger;
    public:
        Ref<spdlog::logger>& Get() { return CoreLogger; }
        void Init() {
            spdlog::set_pattern("%^[%T] %n: %v%$");
            CoreLogger = spdlog::stdout_color_mt("APP");
            CoreLogger->set_level(spdlog::level::trace);
        }
    };
    Ref<spdlog::logger>& Log::CoreLogger;
}
// Log macros
//#define WC_TRACE(...)    ::Hazel::Log::GetLogger()->trace(__VA_ARGS__)
//#define WC_INFO(...)     ::Hazel::Log::GetLogger()->info(__VA_ARGS__)
//#define WC_WARN(...)     ::Hazel::Log::GetLogger()->warn(__VA_ARGS__)
//#define WC_ERROR(...)    ::Hazel::Log::GetLogger()->error(__VA_ARGS__)
//#define WC_CRITICAL(...) ::Hazel::Log::GetLogger()->critical(__VA_ARGS__)