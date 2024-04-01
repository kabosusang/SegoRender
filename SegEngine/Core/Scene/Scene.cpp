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

void Scene::OnUpdate(Timestep ts){
auto& Vctx = VulkanContext::Instance();

Camera* mainCamera = nullptr;
glm::mat4* CameraTransform = nullptr;

    //Render
    {
        auto view = m_Registry.view<TransformComponent,CameraComponent>();
        for(auto entity : view){
            auto [transform,camera] = view.get<TransformComponent,CameraComponent>(entity);
            
           if(camera.Primary){
               mainCamera = &camera.Camera;
               CameraTransform = &transform.Transform;
               break;
           }

        }
    }

    if(mainCamera){
        Vctx.GetRenderer()->BeginScene();
        auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for(auto entity : group){
            auto [transform,sprite] = group.get<TransformComponent,SpriteRendererComponent>(entity);
            //Render}
             }
        Vctx.GetRenderer()->Render();
        Vctx.GetRenderer()->EndScene();
    }


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



}
