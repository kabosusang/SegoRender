#pragma once
#include "Sego.hpp"
#include "Core/Vulkan/VulkanTool.hpp"
#include "Core/Scene/Scene.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ContentBrowsPanel.hpp"
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
    bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

    void NewScene();
    void OpenScene();
    void OpenScene(const std::filesystem::path& path);
    void SaveSceneAs();
    
    void OnScenePlay();
    void OnSceneStop();
    //UI Panels
    void UI_Toolbar(); 
private:
    //Renderer
    std::shared_ptr<Sego::Renderer> m_Renderer;

    vk::Sampler m_Cts;
    vk::Sampler m_Dts;
    vk::DescriptorSet m_color_texture_set;
    vk::DescriptorSet m_depth_texture_set;

    glm::vec2 m_viewportsize  = {0.0f,0.0f};
    glm::vec2 m_ViewportBounds[2];

    bool m_ViewportFocused = false, m_ViewportHovered = false;
    
    std::shared_ptr<Scene> m_ActiveScene;
    Entity m_CameraEntity;
    Entity m_HoveredEntity;

    EditorCamera m_EditorCamera;
    int m_GizmoType = -1;
    //Panels
    SceneHierarchyPanel m_SceneHierarchyPanel;
    ContentBrowsPanel m_ContentBrowsPanel;

    enum class SceneState
    {
        Edit = 0,Play = 1,Stop
    };

    SceneState m_SceneState = SceneState::Edit;

    //UI Resources
    std::shared_ptr<ImGuiImage> m_IconPlay;
    std::shared_ptr<ImGuiImage> m_IconStop;
};








}