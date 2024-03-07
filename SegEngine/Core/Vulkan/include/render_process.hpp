#pragma once

#include "vulkan/vulkan.hpp"
#include "shader.hpp"
#include <fstream>

namespace Sego{

class RenderProcess {
public:
    vk::Pipeline graphicsPipeline = nullptr;
    vk::RenderPass renderPass = nullptr;
    vk::PipelineLayout layout = nullptr;

    RenderProcess();
    ~RenderProcess();

    void RecreateGraphicsPipeline(const Shader& shader);
    void RecreateRenderPass();

private:
    vk::PipelineLayout createLayout();
    vk::Pipeline createGraphicsPipeline(const Shader& shader);
    vk::RenderPass createRenderPass();
};

}
