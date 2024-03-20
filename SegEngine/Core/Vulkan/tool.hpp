#pragma once
#include "pch.h"
#include <vulkan/vulkan.hpp>

namespace Sego{
std::vector<char> ReadWholeFile(const std::string& filename);


class Vulkantool{
    public:
        static vk::CommandBuffer beginSingleCommands();
        static void endInstantCommands(vk::CommandBuffer command_buffer);



};

}