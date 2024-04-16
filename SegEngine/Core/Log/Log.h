#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>


#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#endif

#include <cassert>


namespace Sego{

    class Log
    {
    public:
        static void Log_Init();

        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger;}
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger;}

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;

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


// Core log macros
#define SG_CORE_ERROR(...)		::Sego::Log::GetCoreLogger()->error(__VA_ARGS__)
  
#define SG_CORE_WARN(...)		::Sego::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SG_CORE_INFO(...)		::Sego::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SG_CORE_TRACE(...)		::Sego::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SG_CORE_FATAL(...)		::Sego::Log::GetCoreLogger()->fatal(__VA_ARGS__)


//Client log macros
#define SG_ERROR(...)		::Sego::Log::GetClientLogger()->error(__VA_ARGS__)
#define SG_WARN(...)		::Sego::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SG_INFO(...)		::Sego::Log::GetClientLogger()->info(__VA_ARGS__)
#define SG_TRACE(...)		::Sego::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SG_FATAL(...)		::Sego::Log::GetClientLogger()->fatal(__VA_ARGS__)

#define SG_ASSERT(x, ...) { if(!(x)) { SG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); assert(x); } }

