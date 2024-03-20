#pragma once
#include "pch.h"
#include "Core/Vulkan/tool.hpp"
#include "vulkan/vulkan.hpp"

namespace Sego{
    class ShaderManager
    {
    public:
        void Destroy();

        vk::PipelineShaderStageCreateInfo LoadShader(const std::string& filename, 
        vk::ShaderStageFlagBits stage);
    private:
        std::unordered_map<std::string, vk::ShaderModule> shaderModules_;
    };


}