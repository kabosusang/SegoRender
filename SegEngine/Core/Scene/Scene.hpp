#pragma once
#include "Core/Base/Timestep.hpp"
#include <entt.hpp>

namespace Sego{

class Entity;
class Scene{
public: 
    Scene();
    ~Scene();

    Entity CreateEntity(const std::string& name);
    void DestroyEntity(Entity entity);
    
    void OnUpdate(Timestep ts);
    void OnViewportResize(uint32_t width, uint32_t height);
private:
    template<typename T>
    void OnComponentAdded(Entity entity, T& component);
private:
    entt::registry m_Registry;
    uint32_t m_ViewportWidth = 1600,m_ViewportHeight = 1000;

    friend class Entity;
    friend class SceneSerializer;
    friend class SceneHierarchyPanel;
};






}