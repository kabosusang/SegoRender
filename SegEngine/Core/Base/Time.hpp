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

}