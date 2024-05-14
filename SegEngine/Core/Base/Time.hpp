#pragma once

#include <chrono>

namespace Sego {

	class Timer
	{
	public:
		static float GetTime()
		{
			static auto start = std::chrono::steady_clock::now();
			auto now = std::chrono::steady_clock::now();
			std::chrono::duration<float> duration = now - start;
			return duration.count();
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};

	class StopWatch{
		public:
			void start(){
				m_start_time = std::chrono::high_resolution_clock::now();
			}
			long long stopMs(){
				m_end_time = std::chrono::high_resolution_clock::now();
				return std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_start_time).count();
			}
			float stop(){
				m_end_time = std::chrono::high_resolution_clock::now();
				return std::chrono::duration_cast<std::chrono::nanoseconds>(m_end_time - m_start_time).count() * 1e-9f;
			}
		private:
			std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
			std::chrono::time_point<std::chrono::high_resolution_clock> m_end_time;
	};


}