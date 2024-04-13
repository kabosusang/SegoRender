#include "Render.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "Core/Scene/Scene.hpp"


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

uint32_t Renderer::ReadPixel(uint32_t x, uint32_t y){
    return VulkanRhi::Instance().ReadPixel(x,y);
}

vk::ImageView Renderer::GetColorImageView(){
    return VulkanRhi::Instance().getColorImageView();
}

vk::ImageView Renderer::GetDepthImageView()
{
    return VulkanRhi::Instance().getDepthImageView();
}

const std::vector<uint32_t> Squardindices = {
    0, 1, 2, 2, 3, 0
};

void Renderer::DrawSprite(const glm::mat4& transform,SpriteRendererComponent& src,int entityID, std::vector<std::shared_ptr<RenderData>>& SpriteRenderDatas){

std::shared_ptr<SpriteRenderData> Rendata = std::make_shared<SpriteRenderData>();
    std::vector<SpriteVertex> squard = {
    {{-0.5f, -0.5f,0.0f}, src.Color},
    {{0.5f, -0.5f,0.0f}, src.Color},
    {{0.5f, 0.5f,0.0f}, src.Color},
    {{-0.5f, 0.5f,0.0f}, src.Color}
    };

    Vulkantool::createVertexBuffer(sizeof(squard[0])* squard.size(),
    (void*)squard.data(), Rendata->vertexBuffer_);

    Vulkantool::createIndexBuffer(sizeof(Squardindices[0]) * Squardindices.size(),
    (void*)Squardindices.data(), Rendata->indexBuffer_);
    Rendata->Spritemodel= transform;
    Rendata->indexCount_ = Squardindices.size();
    Rendata->EntityID = entityID + 1; //why plus 1 
    SpriteRenderDatas.push_back(Rendata);
}

void Renderer::Render(Scene* scene){
    auto& VCtx =  VulkanRhi::Instance();

    //Render 2D
    std::vector<std::shared_ptr<RenderData>> SpriteRenderDatas;
    auto view = scene->m_Registry.view<TransformComponent,SpriteRendererComponent>();
    for(auto entity : view){
        auto [transform,spriteRenderer] = view.get<TransformComponent,SpriteRendererComponent>(entity);
        DrawSprite(transform.GetTransform(),spriteRenderer,(int)entity,SpriteRenderDatas);
    }
    //Push Renderer
    VCtx.SetRenderDatas(SpriteRenderDatas);
    

    VulkanRhi::Instance().render();
}













}





