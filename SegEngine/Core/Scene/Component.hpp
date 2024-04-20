#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "SceneCamera.hpp"
#include "Core/Base/UUID.hpp"

//asset
#include "resource/asset/Texture2D.hpp"
#include "framework/Render/Render_data.hpp"


namespace Sego{
    struct IDComponent{
        UUID ID;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent{
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag)
            : Tag(tag) {}
    };


    struct TransformComponent{
        glm::vec3 Translation = {0.0f,0.0f,0.0f};
        glm::vec3 Rotation = {0.0f,0.0f,0.0f};
        glm::vec3 Scale = {1.0f,1.0f,1.0f};
        
        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation)
            : Translation(translation) {}
        glm::mat4 GetTransform() const{
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
           
           return glm::translate(glm::mat4(1.0f),Translation) * rotation * glm::scale(glm::mat4(1.0f),Scale);

        }
       
    };

    struct SpriteRendererComponent{
        glm::vec4 Color{1.0f,1.0f,1.0f,1.0f};
        std::shared_ptr<Texture2D> Texture = nullptr;


        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {}

    };

    struct CameraComponent{
        SceneCamera Camera;
        bool Primary = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    //ForWard 
    class ScriptableEntity;
    struct NativeScriptComponent{
        ScriptableEntity* Instance = nullptr;

        ScriptableEntity*(*InstantiateScript)();
        void(*DestroyScript)(NativeScriptComponent*);

        template<typename T>
        void Bind(){
            InstantiateScript = [](){return static_cast<ScriptableEntity*>(new T());};
            DestroyScript = [](NativeScriptComponent* nsc){delete nsc->Instance; 
            nsc->Instance = nullptr;};

        }

    };

    //Physics
    struct Rigidbody2DComponent{
        enum class BodyType{ Static = 0,Dynamic, Kinematic};
        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        //Store the runtime 
        void* RuntimeBody = nullptr;

        Rigidbody2DComponent() = default;
        Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
    };

    struct BoxCollider2Domponent{
        glm::vec2 Offset = {0.0f,0.0f};
        glm::vec2 Size = {0.5f,0.5f};

        //Physics Material
        float Density = 1.0f; // 密度 
        float Friction = 0.2f; // 摩擦力
        float Restitution = 0.0f; // 弹性
        float RestitutionThreshold = 0.5f; //弹性阈值


        //Store the runtime 
        void* RuntimeBody = nullptr;

        BoxCollider2Domponent() = default;
        BoxCollider2Domponent(const BoxCollider2Domponent&) = default;
    };

    //Mesh
    struct MeshComponent{
        std::string name = "nullmesh";
        std::string path;
        std::shared_ptr<StaticMeshRenderData> MeshData = nullptr;
        
        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;
    };

    

    






};