#include "Render.hpp"
#include "framework/Render/Render_data.hpp"
#include "Core/Vulkan/Vulkantool.hpp"

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
    g_spriteRenderData.clear(); //Clear Sprite

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

const std::vector<uint32_t> Squardindices = {
    0, 1, 2, 2, 3, 0
};
//Render Data
void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& color){
    std::shared_ptr<SpriteRenderData> Rendata = std::make_shared<SpriteRenderData>();
    std::vector<SpriteVertex> squard = {
    {{-0.5f, -0.5f,0.0f}, color},
    {{0.5f, -0.5f,0.0f}, color},
    {{0.5f, 0.5f,0.0f}, color},
    {{-0.5f, 0.5f,0.0f}, color}
    };

    Vulkantool::createVertexBuffer(sizeof(squard[0])* squard.size(),
    (void*)squard.data(), Rendata->vertexBuffer_);

    Vulkantool::createIndexBuffer(sizeof(Squardindices[0]) * Squardindices.size(),
    (void*)Squardindices.data(), Rendata->indexBuffer_);

    Rendata->Spritemodel = transform;
    Rendata->indexCount_ = Squardindices.size();
    g_spriteRenderData.emplace_back(Rendata);
}


}

