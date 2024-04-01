#include "Render.hpp"


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

void Renderer::SetClearColor(const glm::vec4& color){
    VulkanRhi::Instance().setClearColor(color);
}

void Renderer::resizeframbuffer(uint32_t w, uint32_t h)
{
    VulkanRhi::Instance().resizeframbuffer(w,h);
}

vk::ImageView Renderer::GetColorImageView(){
    return VulkanRhi::Instance().getColorImageView();
}





}

