#pragma once
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "EditorCamera.hpp"
#include "framework/Render/Render_data.hpp"
#include "Core/Scene/Component.hpp"


namespace Sego{
class Scene;


class Renderer{
public:
    void Init();

    void BeginScene(const Camera& camera, const glm::mat4& transform);
    void BeginScene(const EditorCamera& camera);
    void BeginScene();

    void EndScene();
    void Render(Scene* scene);
    void DrawQuad(const glm::mat4& transform,glm::vec4& color,int entityID, std::vector<std::shared_ptr<RenderData>>& SpriteRenderDatas);
    void DrawQuad(const glm::mat4& transform,glm::vec4& color,std::shared_ptr<Texture2D> texture,int entityID, std::vector<std::shared_ptr<RenderData>>& SpriteRenderDatas);
    void DrawSprite(const glm::mat4& transform,SpriteRendererComponent& src,int entityID, std::vector<std::shared_ptr<RenderData>>& SpriteRenderDatas);

    // Output Function
    void SetClearColor(const glm::vec4& color);
    void resizeframbuffer(uint32_t w,uint32_t h);
    vk::ImageView GetColorImageView();
    vk::ImageView GetDepthImageView();

    uint32_t ReadPixel(uint32_t x, uint32_t y);

private:
    glm::mat4 m_ViewProj;
 
};




}