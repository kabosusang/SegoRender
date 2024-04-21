#include "pch.h"
#include "Scene.hpp"
#include "Component.hpp"
#include "ScriptEntity.hpp"
#include "Core/Vulkan/VulkanContext.hpp"

//Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

namespace Sego{
static b2BodyType Rigibody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodytype){
    switch (bodytype){
        case Rigidbody2DComponent::BodyType::Static: return b2_staticBody;
        case Rigidbody2DComponent::BodyType::Dynamic: return b2_dynamicBody;
        case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
    }
    SG_ASSERT(false , "Unknown BodyType");
    return b2_staticBody;
}


Scene::Scene(){

    
}

Scene::~Scene(){
    
}

template<typename Component>
static void CopyComponent(entt::registry& dst,entt::registry& src,const std::unordered_map<UUID,entt::entity>& enttMap){
    auto view = src.view<Component>();
    for (auto e : view){
        UUID uuid = src.get<IDComponent>(e).ID;
        SG_ASSERT(enttMap.find(uuid) != enttMap.end(),"Entity UUID not found in map")
        entt::entity dstEntity = enttMap.at(uuid);

        auto& component = src.get<Component>(e);
        dst.emplace_or_replace<Component>(dstEntity,component);
    }
    
}

template<typename Component>
static void CopyComponentIfExists(Entity dst,Entity src){
    if (src.HasComponent<Component>()){
        dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
    }

}


std::shared_ptr<Scene> Scene::Copy(const std::shared_ptr<Scene> &other)
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();
    newScene->m_ViewportWidth = other->m_ViewportWidth;
    newScene->m_ViewportHeight = other->m_ViewportHeight;

    auto& srcSceneRegistry = other->m_Registry;
    auto& dstSceneRegistry = newScene->m_Registry;
    //Copy Entities
    std::unordered_map<UUID,entt::entity> enttMap;
    
    auto idView = srcSceneRegistry.view<IDComponent>();
    for (auto e : idView){
        UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
        const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
        Entity newEntity = newScene->CreateEntityWithUUID(uuid,name);
        enttMap[uuid] = (entt::entity)newEntity;
    }

    //Copy Components
    CopyComponent<TransformComponent>(dstSceneRegistry,srcSceneRegistry,enttMap);
    CopyComponent<SpriteRendererComponent>(dstSceneRegistry,srcSceneRegistry,enttMap);
    CopyComponent<CameraComponent>(dstSceneRegistry,srcSceneRegistry,enttMap);
    CopyComponent<NativeScriptComponent>(dstSceneRegistry,srcSceneRegistry,enttMap);
    CopyComponent<Rigidbody2DComponent>(dstSceneRegistry,srcSceneRegistry,enttMap);
    CopyComponent<BoxCollider2Domponent>(dstSceneRegistry,srcSceneRegistry,enttMap);
    CopyComponent<MeshComponent>(dstSceneRegistry,srcSceneRegistry,enttMap);
    return newScene;
}

Entity Scene::CreateEntity(const std::string& name){

    return CreateEntityWithUUID(UUID(),name);
}

Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
{
    Entity entity = {m_Registry.create(),this};
    entity.AddComponent<IDComponent>(uuid);
    entity.AddComponent<TransformComponent>();
    auto& tag = entity.AddComponent<TagComponent>(name);
    tag.Tag = name.empty() ? "Entity" : name;
    return entity;
}

void Scene::DestroyEntity(Entity entity)
{
    m_Registry.destroy(entity);
}

void Scene::OnRuntimeStart(){
    m_PhysicsWorld = new b2World({0.0f,-9.8f});
    //m_PhysicsWorld->

    auto view = m_Registry.view<Rigidbody2DComponent>();
    for (auto e : view){
        Entity entity = { e,this};
        auto& transform = entity.GetComponent<TransformComponent>();
        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

        b2BodyDef bodyDef;
        bodyDef.type = Rigibody2DTypeToBox2DBody(rb2d.Type);
        bodyDef.position.Set(transform.Translation.x,transform.Translation.y);
        bodyDef.angle = transform.Rotation.z;

        b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.FixedRotation);
        rb2d.RuntimeBody = body;

        if (entity.HasComponent<BoxCollider2Domponent>()){
            auto& bc2d = entity.GetComponent<BoxCollider2Domponent>();

            b2PolygonShape boxShape;
            boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x,bc2d.Size.y * transform.Scale.y);
        
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &boxShape;
            fixtureDef.density = bc2d.Density;
            fixtureDef.friction = bc2d.Friction;
            fixtureDef.restitution = bc2d.Restitution;
            body->CreateFixture(&fixtureDef);
        }


    }

}

void Scene::OnRuntimeStop(){
    delete m_PhysicsWorld;
    m_PhysicsWorld = nullptr;   
}

void Scene::OnUpdateRuntime(Timestep ts)
{
    auto &Vctx = VulkanContext::Instance();

    // Update scripts
    {
        m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto &nsc)
                                                      {
        //TODO: Move to Scene::OnPlay
        if(!nsc.Instance){
            nsc.Instance = nsc.InstantiateScript();
            nsc.Instance->m_Entity = Entity{entity,this};
            nsc.Instance->OnCreate();
        }
            nsc.Instance->OnUpdate(ts); });
    }

    // Physics
    {
        const int32_t velocityIterations = 6;
        const int32_t positionIterations = 2;
        m_PhysicsWorld->Step(ts,velocityIterations,positionIterations);

        //ReTrieve transofmr from Box2D
        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view){
            Entity entity = { e,this};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            b2Body* body = (b2Body*)rb2d.RuntimeBody;
            const auto& position = body->GetPosition();
            transform.Translation = {position.x,position.y,transform.Translation.z};
            transform.Rotation.z = body->GetAngle();
        }
    }

    // Render2D
    Camera *mainCamera = nullptr;
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
    if (mainCamera){
        Vctx.GetRenderer()->BeginScene(*mainCamera,CameraTransform);
        Vctx.GetRenderer()->Render(this);
    }else{
        SG_CORE_WARN("No Primary Camera");
    }


    Vctx.GetRenderer()->Render(); //UI Render (Must Call)
}

void Scene::OnUpdateEditor(Timestep ts,EditorCamera& camera){
    auto &Vctx = VulkanContext::Instance();
    Vctx.GetRenderer()->BeginScene(camera);

    Vctx.GetRenderer()->Render(this); //ALL Entity Render
    Vctx.GetRenderer()->Render(); //UI Render (Must Call)
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

void Scene::DuplicateEntity(Entity entity)
{
    std::string name = entity.GetName();
    Entity newEntity = CreateEntity(name);
      //Copy Components
    CopyComponentIfExists<TransformComponent>(newEntity,entity);
    CopyComponentIfExists<SpriteRendererComponent>(newEntity,entity);
    CopyComponentIfExists<CameraComponent>(newEntity,entity);
    CopyComponentIfExists<NativeScriptComponent>(newEntity,entity);
    CopyComponentIfExists<Rigidbody2DComponent>(newEntity,entity);
    CopyComponentIfExists<BoxCollider2Domponent>(newEntity,entity);
    CopyComponentIfExists<MeshComponent>(newEntity,entity);
}

Entity Scene::GetPrimaryCameraEntity(){
    auto view = m_Registry.view<CameraComponent>();
    for(auto entity : view){
        auto& camera = view.get<CameraComponent>(entity);
        if(camera.Primary){
            return Entity{entity,this};
        }
    }
    return {};
}



template<typename T>
void Scene::OnComponentAdded(Entity entity, T& component){
    SG_CORE_WARN("UnKnown Component Added");
}

template<>
void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component){

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

template<>
void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component){

}

template<>
void Scene::OnComponentAdded<BoxCollider2Domponent>(Entity entity, BoxCollider2Domponent& component){

}

template<>
void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component){
    
    
}


}
