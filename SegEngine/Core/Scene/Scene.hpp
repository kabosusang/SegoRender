#pragma once
#include <entt.hpp>

namespace Sego{

class Entity;
class Scene{
public:
    Scene();
    ~Scene();

    Entity CreateEntity(const std::string& name);
    
    void OnUpdate();
private:
    entt::registry m_Registry;

    friend class Entity;
};






}