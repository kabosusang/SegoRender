#pragma once
#include "Scene.hpp"
#include "Component.hpp"
#include "Core/Base/UUID.hpp"


namespace Sego{

    class Entity{
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene);
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args){
            T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this,component);
            return component;
        }

        template<typename T>
        T& GetComponent(){
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent(){
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
        }
        template<typename T>
        void RemoveComponent(){
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }
        
        operator bool() const { return m_EntityHandle != entt::null; }
        operator uint32_t() const {return (uint32_t)m_EntityHandle;}
        operator entt::entity() const {return m_EntityHandle;}

        UUID GetUUID() { return GetComponent<IDComponent>().ID; }

        bool operator==(const Entity& other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const {
            return !(*this == other); 
        }

    private:
        entt::entity m_EntityHandle = {entt::null};
        Scene* m_Scene = nullptr;
    };



}