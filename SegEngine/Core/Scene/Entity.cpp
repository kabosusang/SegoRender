#include "pch.h"
#include "Entity.hpp"

namespace Sego{

Entity::Entity(entt::entity handle, Scene* scene)
    : m_EntityHandle(handle), m_Scene(scene){
    
}


}