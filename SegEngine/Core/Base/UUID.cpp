#include "pch.h"
#include "UUID.hpp"

#include <random>
#include <unordered_map>

static std::unordered_map<Sego::UUID,std::string> m_Map;

static void AddToMap(){
    m_Map[Sego::UUID()] = "Sego";
}

namespace Sego {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	UUID::UUID()
		: m_UUID(s_UniformDistribution(s_Engine))
	{ 
	}

	UUID::UUID(uint64_t uuid)
		: m_UUID(uuid)
	{
	}

}