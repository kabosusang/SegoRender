#include "Render.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"

namespace Sego{

void Renderer::Init(){
}


void Renderer::BeginScene(){
}

void Renderer::EndScene(){
}

void Renderer::Render(){
    VulkanRhi::Instance().render();
}





}

