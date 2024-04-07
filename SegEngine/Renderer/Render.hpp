#pragma once
#include "Renderer/Camera.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"

namespace Sego{

class Renderer{
public:
    void Init();

    void BeginScene(const Camera& camera, const glm::mat4& transform);
    void BeginScene();
    void EndScene();
    void Render();

    // Output Function
    void SetClearColor(const glm::vec4& color);
    void resizeframbuffer(uint32_t w,uint32_t h);
    vk::ImageView GetColorImageView();

    void DrawQuad(const glm::mat4& transform, const glm::vec4& color);

};




}