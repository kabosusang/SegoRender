#pragma once
#include "Sego.hpp"
#include "Core/Vulkan/VulkanTool.hpp"
#include "Core/Scene/Scene.hpp"

namespace Sego{
class EditorLayer : public Layer{

public:
    EditorLayer(){}
    virtual ~EditorLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(Timestep ts) override;
    virtual void OnImGuiRender() override;

    
    void FramBufferResize(float ,float) ;
    void OnEvent(Event& e) override;
private:
    //Renderer
    std::shared_ptr<Sego::Renderer> m_Renderer;

    vk::Sampler m_Cts;
    vk::DescriptorSet m_color_texture_set;
    glm::vec2 m_viewportsize;
    bool m_ViewportFocused = false, m_ViewportHovered = false;
    
    std::shared_ptr<Scene> m_ActiveScene;
    Entity m_CameraEntity;
    
};








}