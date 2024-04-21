#pragma once
#include "Core/Base/Timestep.hpp"
#include "Core/Base/UUID.hpp"
#include "Renderer/EditorCamera.hpp"



#include <entt.hpp>
class b2World;

namespace Sego{

class Entity;
class Scene{
public: 
    Scene();
    ~Scene();

    static std::shared_ptr<Scene> Copy(const std::shared_ptr<Scene>& other);

    Entity CreateEntity(const std::string& name = std::string());
    Entity CreateEntityWithUUID(UUID uuid,const std::string& name = std::string());

    void DestroyEntity(Entity entity);
    
    void OnRuntimeStart();
    void OnRuntimeStop();

    void OnUpdateEditor(Timestep ts,EditorCamera& camera);
    void OnUpdateRuntime(Timestep ts);
    void OnViewportResize(uint32_t width, uint32_t height);

    void DuplicateEntity(Entity entity);

    Entity GetPrimaryCameraEntity();
    entt::registry &GetRegistry() { return m_Registry; }
private:
    template<typename T>
    void OnComponentAdded(Entity entity, T& component);
private:
    entt::registry m_Registry;
    uint32_t m_ViewportWidth = 1600,m_ViewportHeight = 1000;

    b2World* m_PhysicsWorld = nullptr;

    friend class Entity;
    friend class SceneSerializer;
    friend class SceneHierarchyPanel;
    friend class Renderer;
};






}