#include "pch.h"
#include "Scene.hpp"
#include "Component.hpp"
#include "Entity.hpp" 


namespace Sego{
Scene::Scene(){
    entt::entity entity = m_Registry.create();
}

Scene::~Scene(){
    
}

Entity Scene::CreateEntity(const std::string& name){
    Entity entity = Entity(m_Registry.create(),this);
    entity.AddComponent<TransformComponent>();
    auto& tag = entity.AddComponent<TagComponent>(name);
    tag.Tag = name.empty() ? "Entity" : name;
    
    return entity;
    
}

void Scene::OnUpdate(){
    auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
    for(auto entity : group){
        auto [transform,sprite] = group.get<TransformComponent,SpriteRendererComponent>(entity);
        
    }
}


}