#pragma once
#include <iostream>

namespace Sego{

    class SubMesh 
	{
	public:
		uint32_t m_firstIndex_;
		uint32_t m_indexCount_;
		int32_t  m_materialIndex_;
    };
}