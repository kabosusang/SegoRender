#include "Render.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"

namespace Sego{

void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform){
    glm::mat4 viewProj = camera.GetProjectionMatrix() * glm::inverse(transform);

}

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

