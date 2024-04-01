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
    
    void OnUpdate(Timestep ts);
    void OnViewportResize(uint32_t width, uint32_t height);
private:
    entt::registry m_Registry;
    uint32_t m_ViewportWidth = 0,m_ViewportHeight = 0;

    friend class Entity;
};






}