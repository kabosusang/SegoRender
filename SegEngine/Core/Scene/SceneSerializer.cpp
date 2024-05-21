#include "pch.h"
#include "SceneSerializer.hpp"
#include "Entity.hpp"
#include "Component.hpp"

#include <yaml-cpp/yaml.h>
//asset
#include "resource/asset/Import/gltf_import.hpp"

namespace YAML{

    template<>
    struct convert<glm::vec2>{
        static Node encode(const glm::vec2& rhs){
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node& node,glm::vec2& rhs){
            if(!node.IsSequence() || node.size() != 2)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec3>{
        static Node encode(const glm::vec3& rhs){
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node,glm::vec3& rhs){
            if(!node.IsSequence() || node.size() != 3)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>{
        static Node encode(const glm::vec4& rhs){
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node,glm::vec4& rhs){
            if(!node.IsSequence() || node.size() != 4)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

}



namespace Sego{

YAML::Emitter& operator<<(YAML::Emitter& out,const glm::vec2& v){
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out,const glm::vec3& v){
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out,const glm::vec4& v){
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

// BodyTypeString Change
static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType type){
    switch (type){
        case Rigidbody2DComponent::BodyType::Static: return "Static";
        case Rigidbody2DComponent::BodyType::Dynamic: return "Dynamic";
        case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
    }
    SG_CORE_ERROR("Unknown BodyType");
    return {};
}

static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString){

    if (bodyTypeString == "Static")    return Rigidbody2DComponent::BodyType::Static;
    if (bodyTypeString == "Dynamic")   return Rigidbody2DComponent::BodyType::Dynamic;
    if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

    SG_CORE_ERROR("Unknown BodyType");
    return Rigidbody2DComponent::BodyType::Static;
}

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene> &scene)
    : m_Scene(scene){
    
}

static void SerializeEntity(YAML::Emitter& out,Entity entity){
    SG_ASSERT(entity.HasComponent<IDComponent>(),"Entity must have IDComponent")

    out << YAML::BeginMap; //Entity
    out << YAML::Key << "Entity"  << YAML::Value << entity.GetUUID(); 

    if (entity.HasComponent<TagComponent>()){
        out << YAML::Key << "TagComponent";
        out << YAML::BeginMap; //TagComponent

        auto& tag = entity.GetComponent<TagComponent>().Tag;
        out << YAML::Key << "Tag" << YAML::Value << tag;

        out << YAML::EndMap; //TagComponent
    }

    if (entity.HasComponent<TransformComponent>()){
        out << YAML::Key << "TransformComponent";
        out << YAML::BeginMap; //TransformComponent

        auto& tc = entity.GetComponent<TransformComponent>();
        out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
        out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
        out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
        
        out << YAML::EndMap; //TransformComponent
    }

    if (entity.HasComponent<CameraComponent>()){
        
        out << YAML::Key << "CameraComponent";
        out << YAML::BeginMap; //CameraComponent

        auto& cameraComponent = entity.GetComponent<CameraComponent>();
        auto& camera = cameraComponent.Camera;

        out << YAML::Key << "Camera" << YAML::Value ;
        out << YAML::BeginMap; //Camera
        out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
        out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
        out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
        out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
        out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
        out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
        out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
        out << YAML::EndMap; //Camera

        out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
        out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

        out << YAML::EndMap; //CameraComponent
    }

    if(entity.HasComponent<SpriteRendererComponent>()){
        out << YAML::Key << "SpriteRendererComponent";
        out << YAML::BeginMap; //SpriteRendererComponent

        auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
        out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;

        out << YAML::EndMap; //SpriteRendererComponent
    }

    if(entity.HasComponent<Rigidbody2DComponent>()){
        out << YAML::Key << "Rigidbody2DComponent";
        out << YAML::BeginMap; //Rigidbody2DComponent

        auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
        out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
        out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;
        
        out << YAML::EndMap; //Rigidbody2DComponent
    }

    if(entity.HasComponent<BoxCollider2Domponent>()){
        out << YAML::Key << "BoxCollider2Domponent";
        out << YAML::BeginMap; //BoxCollider2Domponent

        auto& bc2dComponent = entity.GetComponent<BoxCollider2Domponent>();
        out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
        out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
        out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
        out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
        out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;
        
        out << YAML::EndMap; //BoxCollider2Domponent
    }

    if(entity.HasComponent<MeshComponent>()){
        out << YAML::Key << "MeshComponent";
        out << YAML::BeginMap; //BoxCollider2Domponent

        auto& bc2dComponent = entity.GetComponent<MeshComponent>();
        out << YAML::Key << "Name" << YAML::Value << bc2dComponent.name;
        out << YAML::Key << "Path" << YAML::Value << bc2dComponent.path;
        out << YAML::EndMap; //BoxCollider2Domponent
    }

    if (entity.HasComponent<DirLightComponent>()){
        out << YAML::Key << "DirLightComponent";
        out << YAML::BeginMap; //DirLightComponent

        auto& dirLiComponent = entity.GetComponent<DirLightComponent>();
        out << YAML::Key << "Direction" << YAML::Value << dirLiComponent.Direction;
        out << YAML::Key << "Intensity" << YAML::Value << dirLiComponent.Intensity;
        out << YAML::EndMap; //BoxCollider2Domponent
    }


        out << YAML::EndMap; //Entity

}

void SceneSerializer::Serialize(const std::string &filepath){
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << "Untitled";
    out << YAML::Value << "Entities" << YAML::Value << YAML::BeginSeq;
    m_Scene->m_Registry.view<entt::entity>().each([&](auto entityID){
        Entity entity = {entityID,m_Scene.get()}; 
        if (!entity)
            return;
        SerializeEntity(out,entity);
    });
    out << YAML::EndSeq;
    out << YAML::EndMap;

    std::ofstream fout(filepath); 
    fout << out.c_str();
}


bool SceneSerializer::Deserialize(const std::string &filepath){
    std::ifstream stream(filepath);
    std::stringstream ss;
    ss << stream.rdbuf();

    YAML::Node data = YAML::Load(ss.str());
    if(!data["Scene"])
        return false;
    std::string sceneName = data["Scene"].as<std::string>();
    SG_CORE_INFO("Deserializing scene '{0}'",sceneName);

    auto entities = data["Entities"];
    if(entities){
        for (auto entity : entities){
            //1231232145436
            uint64_t uuid = entity["Entity"].as<uint64_t>();

            std::string name;
            auto tagComponent = entity["TagComponent"];
            if(tagComponent) name = tagComponent["Tag"].as<std::string>();

            SG_CORE_INFO("Deserialized entity with ID = {0}, name = {1}",uuid,name);

            Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid,name);

            //Transform
            auto transformComponent = entity["TransformComponent"];
            if (transformComponent){
                //Entites always have transforms
                auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                tc.Scale = transformComponent["Scale"].as<glm::vec3>();
            }

            //Camera
            auto cameraComponent = entity["CameraComponent"];
            if(cameraComponent){
                auto& cc = deserializedEntity.AddComponent<CameraComponent>();

                auto cameraProps = cameraComponent["Camera"];
                cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
                cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
                cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
                cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
                cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
                cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());
                
                cc.Primary = cameraComponent["Primary"].as<bool>();
                cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
            }

            //Sprite
            auto spriteRendererComponent = entity["SpriteRendererComponent"];
            if (spriteRendererComponent){

                auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
                src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
            }

            //Rigidbody2D
            auto rb2dComponent = entity["Rigidbody2DComponent"];
            if (rb2dComponent){
                auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
                rb2d.Type = RigidBody2DBodyTypeFromString(rb2dComponent["BodyType"].as<std::string>());
                rb2d.FixedRotation = rb2dComponent["FixedRotation"].as<bool>();
            }

            //BoxCollider2D
            auto bc2dComponent = entity["BoxCollider2Domponent"];
            if (bc2dComponent){
                auto& bc2d = deserializedEntity.AddComponent<BoxCollider2Domponent>();
                bc2d.Offset = bc2dComponent["Offset"].as<glm::vec2>();
                bc2d.Size = bc2dComponent["Size"].as<glm::vec2>();
                bc2d.Density = bc2dComponent["Density"].as<float>();
                bc2d.Restitution = bc2dComponent["Restitution"].as<float>();
                bc2d.RestitutionThreshold = bc2dComponent["RestitutionThreshold"].as<float>();
            }
            //Mesh Component
            auto meshComponent = entity["MeshComponent"];
            if (meshComponent){
                auto& mc = deserializedEntity.AddComponent<MeshComponent>();
                mc.name = meshComponent["Name"].as<std::string>();
                mc.path = meshComponent["Path"].as<std::string>();
                std::future<void> mesh_async = std::async(std::launch::async,[&](){
                    mc.model  = std::make_shared<GltfModel::Model>();
                    mc.model->loadFromFile(mc.path);
                });
                mesh_async.get();
            }

            //DirLight Component
            auto DirComponent = entity["DirLightComponent"];
            if (DirComponent){
                auto& dc = deserializedEntity.AddComponent<DirLightComponent>();
                dc.Direction = DirComponent["Direction"].as<glm::vec3>();
                dc.Intensity = DirComponent["Intensity"].as<float>();
            }


            


        }
   
    }

    return true;
}


void SceneSerializer::SerializeRuntime(const std::string &filepath){
    
}

bool SceneSerializer::DeserializeRuntime(const std::string &filepath){
    return false;
}




}