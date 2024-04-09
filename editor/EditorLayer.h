#pragma once
#include "Sego.hpp"
#include "Core/Vulkan/VulkanTool.hpp"
#include "Core/Scene/Scene.hpp"
#include "Panels/SceneHierarchyPanel.hpp"

#include "Renderer/EditorCamera.hpp"

namespace Sego{
class EditorLayer : public Layer{

public:
    EditorLayer(){}
    virtual ~EditorLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(Timestep ts) override;
    virtual void OnImGuiRender() override;

    
    void FramBufferResize(float ,float);
    void OnEvent(Event& e) override;
private:
    bool OnKeyPressed(KeyPressedEvent& e);

    void NewScene();
    void OpenScene();
    void SaveSceneAs();
private:
    //Renderer
    std::shared_ptr<Sego::Renderer> m_Renderer;

    vk::Sampler m_Cts;
    vk::DescriptorSet m_color_texture_set;
    glm::vec2 m_viewportsize;
    bool m_ViewportFocused = false, m_ViewportHovered = false;
    
    std::shared_ptr<Scene> m_ActiveScene;
    Entity m_CameraEntity;

    EditorCamera m_EditorCamera;
    int m_GizmoType = -1;
    //Panels
    SceneHierarchyPanel m_SceneHierarchyPanel;
    
};








}