#include "Render.hpp"


namespace Sego{

void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform){
    auto& Vctx = VulkanRhi::Instance();
    //glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
    glm::mat4 Proj = camera.GetProjection();
    //glm::mat4 inverseTransform = glm::inverse(transform);
    //glm::vec3 cameraPosition = glm::vec3(transform[3]);

    //Right 
    glm::mat4 view = transform;

    Vctx.setProjection(Proj);
    Vctx.setView(view);
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

