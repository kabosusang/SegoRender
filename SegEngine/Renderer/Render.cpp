#include "Render.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "Core/Scene/Scene.hpp"
//asset
#include "resource/asset/Import/gltf_import.hpp"


struct Renderer2DData{



};

namespace Sego{

void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform){
    m_CameraPos = glm::vec3(transform[3]);

    glm::mat4 proj = camera.GetProjection();
    proj[1][1] *= -1;
    m_ViewProj = proj *  glm::inverse(transform);
}

void Renderer::BeginScene(const EditorCamera &camera){
    auto& Vctx = VulkanRhi::Instance();

    m_CameraPos = camera.GetPosition();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix();

    proj[1][1] *= -1;
    m_ViewProj = proj * view;
    if (camera.m_UseSkybox){
        skybox_->Meshmvp_ = proj * glm::mat4(glm::mat3(view));
        Vctx.SetSkyboxRenderData(skybox_);
    }

}

void Renderer::Init(){
    auto& Vctx = VulkanRhi::Instance();
    uint32_t maxFlightCount_ = Vctx.getMaxFlightCount();
    skybox_ = std::make_shared<SkyboxRenderData>();
    skybox_ = std::static_pointer_cast<SkyboxRenderData>(GlTFImporter::LoadglTFFile("resources/Settings/skybox/cube.gltf"));

    //lightubs
    vk::DeviceSize bufferSize = sizeof(LightObj);
    m_Lightubs_.resize(maxFlightCount_);
    for(int i = 0; i < maxFlightCount_; ++i){
        Vulkantool::createBuffer(bufferSize,vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU, m_Lightubs_[i]);
    }

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

void Renderer::DrawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade, int entityID)
{
    
}

void Renderer::Render(Scene* scene){
    auto& VCtx =  VulkanRhi::Instance();

    //Lighting
    LightObj light;
    std::shared_ptr<LightingRenderData> light_data = std::make_shared<LightingRenderData>();
    light.lightSetting.UseLight = 0; //No Light
    light.lightSetting.lightCount = 0;
    light.dirLight.direction = glm::vec3(0.0f,0.0f,0.0f);
    light.dirLight.viewPos = m_CameraPos;
    //shadowmap
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    auto DirLightview = scene->m_Registry.view<TransformComponent,DirLightComponent>();
    for (auto entity : DirLightview){
        auto [transform,dirLight] = DirLightview.get<TransformComponent,DirLightComponent>(entity);
        light.dirLight.direction = dirLight.Direction;
        light.lightSetting.UseLight = 1; //Use Light
        light.lightSetting.lightCount = 1;
        //shadow
        float near_plane = 0.1f, far_plane = 1000.0f;
        glm::vec3 lightPos = -dirLight.Direction * 10.0f;
        glm::vec3 lightTarget = glm::vec3(0.0f);
        glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), 1.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        lightProjection[1][1] *= -1;
        lightSpaceMatrix = lightProjection * lightView;
    }
    light_data->camera_view_proj = lightSpaceMatrix;
    light_data->directional_light_shadow_texture = VCtx.getDirShadowMap();
    
    VCtx.updateShadowubos(lightSpaceMatrix);

    //update light uniform buffers
    VmaBuffer ubs = m_Lightubs_[VCtx.getFlightCount()];
    Vulkantool::updateBuffer(ubs,&light,sizeof(LightObj));
    light_data->lighting_ubs = m_Lightubs_;
    VCtx.SetLightRenderData(light_data);
      
    //Render 2D
    std::vector<std::shared_ptr<RenderData>> RenderDatas;
    auto view = scene->m_Registry.view<TransformComponent,SpriteRendererComponent>();
    for(auto entity : view){
        auto [transform,spriteRenderer] = view.get<TransformComponent,SpriteRendererComponent>(entity);
        DrawSprite(transform.GetTransform(),spriteRenderer,(int)entity,RenderDatas);
    }

    //Render 3D Static Mesh
    auto view2 = scene->m_Registry.view<TransformComponent,MeshComponent>();
    for(auto entity : view2){
        auto [transform,meshRenderer] = view2.get<TransformComponent,MeshComponent>(entity);
        if (meshRenderer.MeshData == nullptr)
            continue;
        meshRenderer.MeshData->model_ = transform.GetTransform();
        meshRenderer.MeshData->Meshmvp_ = m_ViewProj * transform.GetTransform();
        meshRenderer.MeshData->EntityID = (int)entity + 1;
        RenderDatas.push_back(meshRenderer.MeshData);
    }

    //Push Renderer

    VCtx.SetRenderDatas(RenderDatas);
}

void Renderer::Render(){
    auto& VCtx =  VulkanRhi::Instance();
    VCtx.render();
}












}





