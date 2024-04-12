#include "Render.hpp"
#include "framework/Render/Render_data.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/Component.hpp"

namespace Sego{

void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform){
    auto& Vctx = VulkanRhi::Instance();
    //glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
    glm::mat4 Proj = camera.GetProjection();
    //glm::mat4 inverseTransform = glm::inverse(transform);
    //glm::vec3 cameraPosition = glm::vec3(transform[3]);
    //Right 
    glm::mat4 view = glm::inverse(transform);
    m_ViewProj = Proj * view;
}

void Renderer::BeginScene(const EditorCamera &camera){
auto& Vctx = VulkanRhi::Instance();
//glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 proj = camera.GetProjectionMatrix();

Vctx.setView(view);
Vctx.setProjection(proj);

}

void Renderer::Init(){
   
}

void Renderer::BeginScene(){
}

void Renderer::EndScene(){

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

void Renderer::Render(Scene* scene){
    auto& VCtx =  VulkanRhi::Instance();

    //Render 2D
    std::vector<std::shared_ptr<RenderData>> SpriteRenderDatas;
    auto view = scene->m_Registry.view<TransformComponent,SpriteRendererComponent>();
    for(auto entity : view){
        auto [transform,spriteRenderer] = view.get<TransformComponent,SpriteRendererComponent>(entity);
        std::shared_ptr<SpriteRenderData> Rendata = std::make_shared<SpriteRenderData>();
        std::vector<SpriteVertex> squard = {
        {{-0.5f, -0.5f,0.0f}, spriteRenderer.Color},
        {{0.5f, -0.5f,0.0f}, spriteRenderer.Color},
        {{0.5f, 0.5f,0.0f}, spriteRenderer.Color},
        {{-0.5f, 0.5f,0.0f}, spriteRenderer.Color}
        };

        Vulkantool::createVertexBuffer(sizeof(squard[0])* squard.size(),
        (void*)squard.data(), Rendata->vertexBuffer_);

        Vulkantool::createIndexBuffer(sizeof(Squardindices[0]) * Squardindices.size(),
        (void*)Squardindices.data(), Rendata->indexBuffer_);
        Rendata->Spritemodel= transform.GetTransform();

        Rendata->indexCount_ = Squardindices.size();
        SpriteRenderDatas.push_back(Rendata);
    }

    //Push Renderer
    VCtx.SetRenderDatas(SpriteRenderDatas);
    

    VulkanRhi::Instance().render();
}












}





