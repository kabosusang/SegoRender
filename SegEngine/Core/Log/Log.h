#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/ringbuffer_sink.h>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#endif

#include <cassert>

namespace Sego{
    enum class ELogLevel
	{
		Debug, Info, Warning, Error, Fatal
	};

    class Log
    {
    public:
        static void Log_Init();
        static void destroy();
    
        static std::vector<std::string> getLastestLogs();
        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger;}
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger;}

        template<typename... TARGS>
        static void log(ELogLevel level, TARGS&&... args)
            {
                switch (level)
                {
				case ELogLevel::Debug:
					s_CoreLogger->debug(std::forward<TARGS>(args)...);
					break;
				case ELogLevel::Info:
					s_CoreLogger->info(std::forward<TARGS>(args)...);
					break;
				case ELogLevel::Warning:
					s_CoreLogger->warn(std::forward<TARGS>(args)...);
					break;
				case ELogLevel::Error:
					s_CoreLogger->error(std::forward<TARGS>(args)...);
					break;
				case ELogLevel::Fatal:
				{
					s_CoreLogger->critical(args...);

					// throw application runtime error
					std::string fatal_str = fmt::format(std::forward<TARGS>(args)...);
					throw std::runtime_error(fatal_str);
				}
					break;
				default:
					break;
                }
            }


    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
        static std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> s_ringbuffer_sink;
    };

}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

#define LOG_HELPER(LOG_LEVEL, ...) \
    Log::log(LOG_LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__)

// Core log macros
#define SG_CORE_ERROR(...)		LOG_HELPER(ELogLevel::Error, __VA_ARGS__)
  
#define SG_CORE_WARN(...)		LOG_HELPER(ELogLevel::Warning, __VA_ARGS__)
#define SG_CORE_INFO(...)		LOG_HELPER(ELogLevel::Info, __VA_ARGS__)
#define SG_CORE_TRACE(...)		LOG_HELPER(ELogLevel::Debug, __VA_ARGS__)
#define SG_CORE_FATAL(...)		LOG_HELPER(ELogLevel::Fatal, __VA_ARGS__)


//Client log macros
#define SG_ERROR(...)		::Sego::Log::GetClientLogger()->error(__VA_ARGS__)
#define SG_WARN(...)		::Sego::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SG_INFO(...)		::Sego::Log::GetClientLogger()->info(__VA_ARGS__)
#define SG_TRACE(...)		::Sego::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SG_FATAL(...)		::Sego::Log::GetClientLogger()->fatal(__VA_ARGS__)

#define SG_ASSERT(x, ...) { if(!(x)) { SG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); assert(x); } }

