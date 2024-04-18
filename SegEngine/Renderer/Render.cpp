#include "Render.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "Core/Scene/Scene.hpp"

namespace Sego{

void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform){
    glm::mat4 proj = camera.GetProjection();
     proj[1][1] *= -1;
    m_ViewProj = proj *  glm::inverse(transform);
}

void Renderer::BeginScene(const EditorCamera &camera){
auto& Vctx = VulkanRhi::Instance();
//glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 proj = camera.GetProjectionMatrix();

proj[1][1] *= -1;
m_ViewProj = proj * view;





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

void Renderer::DrawQuad(const glm::mat4& transform,glm::vec4& color,int entityID, std::vector<std::shared_ptr<RenderData>>& SpriteRenderDatas){
    std::shared_ptr<SpriteRenderData> Rendata = std::make_shared<SpriteRenderData>();
    std::vector<SpriteVertex> squard = {
    {{-0.5f, -0.5f,0.0f}, color,{0.0f, 0.0f}},
    {{0.5f, -0.5f,0.0f}, color,{1.0f, 0.0f}},
    {{0.5f, 0.5f,0.0f},color,{1.0f, 1.0f}},
    {{-0.5f, 0.5f,0.0f}, color,{0.0f, 1.0f}}
    };

    Vulkantool::createVertexBuffer(sizeof(squard[0])* squard.size(),
    (void*)squard.data(), Rendata->vertexBuffer_);

    Vulkantool::createIndexBuffer(sizeof(Squardindices[0]) * Squardindices.size(),
    (void*)Squardindices.data(), Rendata->indexBuffer_);
    Rendata->Spritemvp_= m_ViewProj * transform;
    Rendata->UseTex = 0; 
    
    Rendata->indexCount_ = Squardindices.size();
    Rendata->EntityID = entityID + 1; //why plus 1 
    SpriteRenderDatas.push_back(Rendata);
}

void Renderer::DrawQuad(const glm::mat4& transform,glm::vec4& color,std::shared_ptr<Texture2D> texture,int entityID, std::vector<std::shared_ptr<RenderData>>& SpriteRenderDatas){
    std::shared_ptr<SpriteRenderData> Rendata = std::make_shared<SpriteRenderData>();
    std::vector<SpriteVertex> squard = {
    {{-0.5f, -0.5f,0.0f}, color,{0.0f, 0.0f}},
    {{0.5f, -0.5f,0.0f}, color,{1.0f, 0.0f}},
    {{0.5f, 0.5f,0.0f},color,{1.0f, 1.0f}},
    {{-0.5f, 0.5f,0.0f}, color,{0.0f, 1.0f}}
    };

    Vulkantool::createVertexBuffer(sizeof(squard[0])* squard.size(),
    (void*)squard.data(), Rendata->vertexBuffer_);

    Vulkantool::createIndexBuffer(sizeof(Squardindices[0]) * Squardindices.size(),
    (void*)Squardindices.data(), Rendata->indexBuffer_);
    Rendata->Spritemvp_= m_ViewProj * transform;
    Rendata->Spritetexture = texture;
    Rendata->UseTex  = 1;

    Rendata->indexCount_ = Squardindices.size();
    Rendata->EntityID = entityID + 1; //why plus 1 
    SpriteRenderDatas.push_back(Rendata);
}

void Renderer::DrawSprite(const glm::mat4& transform,SpriteRendererComponent& src,int entityID, std::vector<std::shared_ptr<RenderData>>& SpriteRenderDatas){
    if (src.Texture)
        DrawQuad(transform,src.Color,src.Texture,entityID,SpriteRenderDatas);
    else
        DrawQuad(transform,src.Color,entityID,SpriteRenderDatas);
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
    SpriteRenderDatas.clear();
}

void Renderer::Render(){
    auto& VCtx =  VulkanRhi::Instance();
    VCtx.render();
}












}





