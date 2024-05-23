#include "Render.hpp"
#include "Core/Vulkan/Vulkantool.hpp"
#include "Core/Scene/Scene.hpp"
//asset
#include "resource/asset/Import/gltf_import.hpp"


struct Renderer2DData{



};

namespace Sego{

void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform){
    auto& Vctx = VulkanRhi::Instance();
    m_CameraPos = glm::vec3(transform[3]);

    glm::mat4 proj = camera.GetProjection();
    proj[1][1] *= -1;
    m_ViewProj = proj *  glm::inverse(transform);
    Vctx.m_ViewMatrix = glm::inverse(transform);
    Vctx.m_ProjectionMatrix = proj;
}

void Renderer::BeginScene(const EditorCamera &camera){
    auto& Vctx = VulkanRhi::Instance();
    m_Camenear = camera.GetCmaeraNearClip();
    m_Camefar = camera.GetCmaeraFarClip();
    m_CameraPos = camera.GetPosition();
    m_CameraDir = camera.GetDirPoint();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix();

    proj[1][1] *= -1;
    Vctx.m_ViewMatrix = view;
    Vctx.m_ProjectionMatrix = proj;
    m_ViewProj = proj * view;
    SkyboxMvp_ = proj * glm::mat4(glm::mat3(view));
    //Vctx.SetSkyboxRenderData(m_skybox);
}

void Renderer::Init(){
    auto& Vctx = VulkanRhi::Instance();
    uint32_t maxFlightCount_ = Vctx.getMaxFlightCount();

    //lightubs
    vk::DeviceSize bufferSize = sizeof(LightingUBO);
    m_Lightubs_.resize(maxFlightCount_);
    for(int i = 0; i < maxFlightCount_; ++i){
        Vulkantool::createBuffer(bufferSize,vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU, m_Lightubs_[i]);
    }

}

void Renderer::destory(){
    for (auto& buffer : m_Lightubs_){
        buffer.destroy();
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
    // set render datas
    const VmaImageViewSampler& default_texture_2d = VCtx.defaultTexture->image_view_sampler_;
    std::shared_ptr<LightingRenderData> lighting_render_data = std::make_shared<LightingRenderData>();
    lighting_render_data->camera_view_proj = m_ViewProj;
    lighting_render_data->brdf_lut_texture = default_texture_2d;
    lighting_render_data->irradiance_texture = default_texture_2d;
    lighting_render_data->prefilter_texture =default_texture_2d;
    lighting_render_data->directional_light_shadow_texture = VCtx.getDirShadowMap();
    lighting_render_data->point_light_shadow_textures.resize(MAX_POINT_LIGHT_NUM);
    lighting_render_data->spot_light_shadow_textures.resize(MAX_SPOT_LIGHT_NUM);

    for (uint32_t i = 0; i < MAX_POINT_LIGHT_NUM; ++i)
    {
        lighting_render_data->point_light_shadow_textures[i] = default_texture_2d;
    }
    for (uint32_t i = 0; i < MAX_SPOT_LIGHT_NUM; ++i)
    {
        lighting_render_data->spot_light_shadow_textures[i] = default_texture_2d;
    }
    //skybox
    std::shared_ptr<SkyboxRenderData> skybox_render_data = nullptr;

    // shadow create infos
    ShadowCascadeCreateInfo shadow_cascade_ci{};
    shadow_cascade_ci.camera_near = m_Camenear;
    shadow_cascade_ci.camera_far = m_Camefar;
    shadow_cascade_ci.inv_camera_view_proj = glm::inverse(m_ViewProj);

    std::vector<ShadowCubeCreateInfo> shadow_cube_cis;
	std::vector<ShadowFrustumCreateInfo> shadow_frustum_cis;

    // set lighting uniform buffer object
    LightingUBO lighting_ubo;
    lighting_ubo.camera_pos = m_CameraPos;
    lighting_ubo.exposure = 4.5f; // TODO
    lighting_ubo.camera_view = VCtx.m_ViewMatrix;
    lighting_ubo.inv_camera_view_proj = glm::inverse(m_ViewProj);
    lighting_ubo.has_sky_light = lighting_ubo.has_directional_light = false;
    lighting_ubo.point_light_num = lighting_ubo.spot_light_num = 0;

    auto DirLightview = scene->m_Registry.view<TransformComponent,DirLightComponent>();
    for (auto entity : DirLightview){
        auto [transform,dirLight] = DirLightview.get<TransformComponent,DirLightComponent>(entity);
       
        // set lighting uniform buffer object
        lighting_ubo.has_directional_light = true;
        lighting_ubo.directional_light.direction = dirLight.Direction;
        lighting_ubo.directional_light.color = dirLight.Color;
        lighting_ubo.directional_light.cast_shadow = dirLight.castshadow;

        shadow_cascade_ci.light_dir = dirLight.Direction;
        shadow_cascade_ci.light_cascade_frustum_near = dirLight.m_cascade_frustum_near;
    }
    // get sky light component
    auto SkyLightview = scene->m_Registry.view<TransformComponent,SkyLightComponent>();
    for (auto entity : SkyLightview){
        auto [transform,skyLight] = SkyLightview.get<TransformComponent,SkyLightComponent>(entity);

        // set lighting render data
        if (skyLight.textureCube == nullptr) continue;
        lighting_render_data->brdf_lut_texture = skyLight.skylight->lutBrdfIVs_;
        lighting_render_data->irradiance_texture = skyLight.skylight->irradianceIVs_;
        lighting_render_data->prefilter_texture = skyLight.skylight->prefilteredIVs_;

        // set skybox render data
        skybox_render_data = std::make_shared<SkyboxRenderData>();
        skybox_render_data->box_ = skyLight.skylight->cube_mesh;
        skybox_render_data->Meshmvp_ = SkyboxMvp_;
        skybox_render_data->env_texture = skyLight.skylight->prefilteredIVs_;

        // set lighting uniform buffer object
        lighting_ubo.has_sky_light = true;
        lighting_ubo.sky_light.color = skyLight.Color;
        lighting_ubo.sky_light.prefilter_mip_levels = skyLight.skylight->m_prefilter_mip_levels;
    }
    //PointLight Component
    auto PointtLightview = scene->m_Registry.view<TransformComponent,PointLightComponent>();
    for (auto entity : PointtLightview){
        auto [transform,pointLight] = PointtLightview.get<TransformComponent,PointLightComponent>(entity);

        // set lighting uniform buffer object
        PointLight& point_light = lighting_ubo.point_lights[lighting_ubo.point_light_num++];
        point_light.position = transform.Translation;
        point_light.color = pointLight.Color;
        point_light.radius = pointLight.m_radius;
        point_light.linear_attenuation = pointLight.m_linear_attenuation;
        point_light.quadratic_attenuation = pointLight.m_quadratic_attenuation;
        point_light.cast_shadow = pointLight.castshadow;

        ShadowCubeCreateInfo shadow_cube_ci;
        shadow_cube_ci.light_pos = transform.Translation;
        shadow_cube_ci.light_far = pointLight.m_radius;
        shadow_cube_ci.light_near = m_Camenear;
        shadow_cube_cis.push_back(shadow_cube_ci);
    }
    //Spot Light Component
    auto SpotLightview = scene->m_Registry.view<TransformComponent,SpoitLightComponent>();
    for (auto entity : SpotLightview){
        auto [transform,spotlight] = SpotLightview.get<TransformComponent,SpoitLightComponent>(entity);
        // set lighting uniform buffer object
        SpotLight& spot_light = lighting_ubo.spot_lights[lighting_ubo.spot_light_num++];
        PointLight& point_light = spot_light._pl;
        point_light.position = transform.Translation;
        point_light.color = spotlight.Color;
        point_light.radius = spotlight.m_radius;
        point_light.linear_attenuation = spotlight.m_linear_attenuation;
        point_light.quadratic_attenuation = spotlight.m_quadratic_attenuation;
        point_light.cast_shadow = spotlight.castshadow;
        point_light.padding0 = std::cos(glm::radians(spotlight.m_inner_cone_angle));
        point_light.padding1 = std::cos(glm::radians(spotlight.m_outer_cone_angle));

        spot_light.direction = spotlight.Direction;
        ShadowFrustumCreateInfo shadow_frustum_ci;
        shadow_frustum_ci.light_pos =  transform.Translation;
        shadow_frustum_ci.light_dir = spot_light.direction;
        shadow_frustum_ci.light_angle = spotlight.m_outer_cone_angle;
        shadow_frustum_ci.light_far = spotlight.m_radius;
        shadow_frustum_ci.light_near = m_Camenear;
        shadow_frustum_cis.push_back(shadow_frustum_ci);
    }

    //Render 2D
    std::vector<std::shared_ptr<RenderData>> RenderDatas;
   
    auto view = scene->m_Registry.view<TransformComponent,SpriteRendererComponent>();
    for(auto entity : view){
        auto [transform,spriteRenderer] = view.get<TransformComponent,SpriteRendererComponent>(entity);
        DrawSprite(transform.GetTransform(),spriteRenderer,(int)entity,RenderDatas);
    }
    //Render 3D Static Mesh
    
    auto view2 = scene->m_Registry.view<TransformComponent,MeshComponent,MaterialComponent>();
    for(auto entity : view2){
        auto [transform,meshRenderer,material] = view2.get<TransformComponent,MeshComponent,MaterialComponent>(entity);
        std::shared_ptr<PbrMeshRenderData> MesData = nullptr;
        if (meshRenderer.model == nullptr)
            continue;
        MesData = std::make_shared<PbrMeshRenderData>();
        MesData->model_ = transform.GetTransform();
        MesData->EntityID = (int)entity + 1;
        MesData->model = meshRenderer.model;

        //Material
        
        MesData->MaterialBuffer = material.shaderMaterialBuffer;
        RenderDatas.push_back(MesData);
    }


    // directional light shadow pass: n mesh datas
    if (lighting_ubo.has_directional_light)
    {
        VCtx.dirPass_->updateCascades(shadow_cascade_ci);
        
        for (uint32_t i = 0; i < SHADOW_CASCADE_NUM; ++i)
        {
            lighting_ubo.directional_light.cascade_splits[i] = VCtx.dirPass_->m_cascade_splits[i];
            lighting_ubo.directional_light.cascade_view_projs[i] = VCtx.dirPass_->m_shadow_cascade_ubo.cascade_view_projs[i];
        }

        if (lighting_ubo.directional_light.cast_shadow)
        {
            VCtx.dirPass_->setRenderDatas(RenderDatas);
        }
    }

    // point light shadow pass: n mesh datas
    if (lighting_ubo.point_light_num > 0)
    {
         VCtx.pointPass_->updateCubes(shadow_cube_cis);
        const auto& point_light_shadow_textures = VCtx.pointPass_->getShadowImageViewSamplers();
        for (size_t i = 0; i < point_light_shadow_textures.size(); ++i)
        {
            lighting_render_data->point_light_shadow_textures[i] = point_light_shadow_textures[i];
        }

        bool cast_shadow = false;
        for (uint32_t i = 0; i < lighting_ubo.point_light_num; ++i)
        {
            if (lighting_ubo.point_lights[i].cast_shadow)
            {
                cast_shadow = true;
                break;
            }
        }

        if (cast_shadow)
        {
            VCtx.pointPass_->setRenderDatas(RenderDatas);
        }
    }

    // spot light shadow pass: n mesh datas
    if (lighting_ubo.spot_light_num > 0)
    {
        VCtx.spotPass_->updateFrustums(shadow_frustum_cis);
        const auto& spot_light_shadow_textures = VCtx.spotPass_->getShadowImageViewSamplers();
        for (size_t i = 0; i < spot_light_shadow_textures.size(); ++i)
        {
            lighting_render_data->spot_light_shadow_textures[i] = spot_light_shadow_textures[i];
        }

        bool cast_shadow = false;
        for (uint32_t i = 0; i < lighting_ubo.spot_light_num; ++i)
        {
            lighting_ubo.spot_lights[i].view_proj = VCtx.spotPass_->m_light_view_projs[i];
            if (lighting_ubo.spot_lights[i]._pl.cast_shadow)
            {
                cast_shadow = true;
            }
        }

        if (cast_shadow)
        {
            VCtx.spotPass_->setRenderDatas(RenderDatas);
        }
    }

    // update lighting uniform buffers
    VmaBuffer uniform_buffer = m_Lightubs_[VCtx.getFlightCount()];
    Vulkantool::updateBuffer(uniform_buffer, (void*)&lighting_ubo, sizeof(LightingUBO));
    lighting_render_data->lighting_ubs = m_Lightubs_;

    //Push Renderer
    VCtx.SetLightRenderData(lighting_render_data);
    VCtx.SetSkyboxRenderData(skybox_render_data);
    VCtx.SetRenderDatas(RenderDatas);

}

void Renderer::Render(){
    auto& VCtx =  VulkanRhi::Instance();
    VCtx.render();
}




}





