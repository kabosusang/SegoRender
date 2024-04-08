#pragma once

#include "Core/Scene/Scene.hpp"
#include "Core/Scene/Entity.hpp"

namespace Sego{

class SceneHierarchyPanel{
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const std::shared_ptr<Scene>& scene);

        void SetContext(const std::shared_ptr<Scene>& scene);

        void OnImGuiRender();

        Entity GetSelectedEntity() const { return m_SelectionContext; }
    private:
        void DrawEnityNode(Entity entity);
        void DrawComponents(Entity entity);
    private:
        std::shared_ptr<Scene> m_Context;
        Entity m_SelectionContext;
};





}