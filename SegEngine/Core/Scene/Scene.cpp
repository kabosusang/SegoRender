#include "pch.h"
#include "Scene.hpp"
#include "Component.hpp"
#include "Entity.hpp" 
#include "Core/Vulkan/VulkanContext.hpp"

namespace Sego{
Scene::Scene(){

    
}

Scene::~Scene(){
    
}

Entity Scene::CreateEntity(const std::string& name){
    Entity entity = {m_Registry.create(),this};
    entity.AddComponent<TransformComponent>();
    auto& tag = entity.AddComponent<TagComponent>(name);
    tag.Tag = name.empty() ? "Entity" : name;
    return entity;
}

void Scene::DestroyEntity(Entity entity){
    m_Registry.destroy(entity);
}

void Scene::OnUpdate(Timestep ts){
auto& Vctx = VulkanContext::Instance();

//Update scripts
{
    m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc){
        //TODO: Move to Scene::OnPlay
        if(!nsc.Instance){
            nsc.Instance = nsc.InstantiateScript();
            nsc.Instance->m_Entity = Entity{entity,this};
            nsc.Instance->OnCreate();
        }
            nsc.Instance->OnUpdate(ts);
    });
}

//Render3D
Camera* mainCamera = nullptr;
glm::mat4 CameraTransform;

    //Render
    {
        auto view = m_Registry.view<TransformComponent,CameraComponent>();
        for(auto entity : view){
            auto [transform,camera] = view.get<TransformComponent,CameraComponent>(entity);
            
           if(camera.Primary){
               mainCamera = &camera.Camera;
               CameraTransform = transform.GetTransform();
               break;
           }

        }
    }

    if(mainCamera){
        Vctx.GetRenderer()->BeginScene(mainCamera->GetProjection(),CameraTransform);
        auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for(auto entity : group){
            auto [transform,sprite] = group.get<TransformComponent,SpriteRendererComponent>(entity);
            Vctx.GetRenderer()->DrawQuad(transform.GetTransform(),sprite.Color); //TODO: Add texture
            
        }
    }
    Vctx.GetRenderer()->Render();
    Vctx.GetRenderer()->EndScene();
}

void Scene::OnViewportResize(uint32_t width, uint32_t height){
    m_ViewportWidth = width;
    m_ViewportHeight = height;

    //Resize cameras
    auto view = m_Registry.view<CameraComponent>();
    for (auto entity : view){
        auto& cameraComponent = view.get<CameraComponent>(entity);
        if(!cameraComponent.FixedAspectRatio){
            cameraComponent.Camera.SetViewportSize(width,height);
        }
    }
    
}

template<typename T>
void Scene::OnComponentAdded(Entity entity, T& component){
    SG_CORE_WARN("UnKnown Component Added");
}

template<>
void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component){

}

template<>
void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component){
    component.Camera.SetViewportSize(m_ViewportWidth,m_ViewportHeight);

}

template<>
void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component){

}

template<>
void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component){

}

template<>
void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component){

}





}
