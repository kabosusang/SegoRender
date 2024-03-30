#pragma once
#include "pch.h"

namespace Sego{
    struct Primitive {
		uint32_t firstIndex;
		uint32_t indexCount;
		int32_t materialIndex;
	};

    struct Mesh {
		std::vector<Primitive> primitives;
	};




}