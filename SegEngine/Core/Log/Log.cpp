#include "Log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>


#define MAX_RINGBUFFER_SIZE 100


namespace Sego{
    
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> Log::s_ringbuffer_sink;

    void Log::Log_Init()
	{
		//OLd 
		//spdlog::set_pattern("%^[%T] %n: %v%$");
		//s_CoreLogger = spdlog::stdout_color_mt("Sego");
		//s_CoreLogger->set_level(spdlog::level::trace);
		//s_ClientLogger = spdlog::stdout_color_mt("Edit");
		//s_ClientLogger->set_level(spdlog::level::trace);

		// CONSOLE sink
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::trace);
		console_sink->set_pattern("[%^%l%$] %v");


		// ring-buffer sink
		s_ringbuffer_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(MAX_RINGBUFFER_SIZE);
		s_ringbuffer_sink->set_level(spdlog::level::trace);
		s_ringbuffer_sink->set_pattern("[%l] %v");


 	const spdlog::sinks_init_list sink_list = { console_sink, s_ringbuffer_sink };

		// create multi-sink logger
        spdlog::init_thread_pool(8192, 1);

		s_CoreLogger = std::make_shared<spdlog::async_logger>("multi_sink_logger",
            sink_list.begin(), sink_list.end(), spdlog::thread_pool(),
            spdlog::async_overflow_policy::block);
        s_CoreLogger->set_level(spdlog::level::trace);

        spdlog::register_logger(s_CoreLogger);
}

void Log::destroy()
{
	s_ClientLogger->flush();
	s_CoreLogger->flush();
    spdlog::drop_all();
}

std::vector<std::string> Log::getLastestLogs()
{
    return s_ringbuffer_sink->last_formatted();
}




}