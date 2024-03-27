#pragma once
#include "Sego.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "Core/Vulkan/VulkanTool.hpp"

namespace Sego{
class EditorLayer : public Layer{

public:
    EditorLayer(){}
    virtual ~EditorLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnImGuiRender() override;

    virtual void OnResize() override;
    void OnEvent(Event& e) override;
private:
    vk::Sampler m_Cts;
    vk::DescriptorSet m_color_texture_set;
    glm::vec2 m_viewportsize;
        
};








}