#include "toy2d/toy2d.hpp"

namespace toy2d {

void Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func, int w, int h) {
    Context::Init(extensions, func);
    Context::GetInstance().InitSwapchain(w, h);
    Shader::Init(ReadWholeFile("./shader/vert.spv"), ReadWholeFile("./shader/frag.spv"));
    Context::GetInstance().renderProcess->InitRenderPass();
    Context::GetInstance().renderProcess->InitLayout();
    Context::GetInstance().swapchain->CreateFramebuffers(w, h);
    Context::GetInstance().renderProcess->InitPipeline(w, h);
    Context::GetInstance().InitRenderer();
}

void Quit() {
    Context::GetInstance().device.waitIdle();
    Context::GetInstance().renderer.reset();
    Context::GetInstance().renderProcess.reset();
    Context::GetInstance().DestroySwapchain();
    Shader::Quit();
    Context::Quit();
}


std::vector<vk::PipelineShaderStageCreateInfo> Shader::GetStage() {
    return stage_;
}

}