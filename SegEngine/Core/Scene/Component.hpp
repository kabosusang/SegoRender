#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "SceneCamera.hpp"
#include "Core/Base/UUID.hpp"

//asset
#include "framework/Render/Render_data.hpp"
#include "resource/asset/Texture2D.hpp"
#include "resource/asset/CubeTexture.hpp"
#include "resource/asset/Import/gltf_import.hpp"


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

    struct CircleRendererComponent{
        glm::vec4 Color{1.0f,1.0f,1.0f,1.0f};
        float Thickness = 1.0f;
        float Fade = 0.005f;
    
        CircleRendererComponent() = default;
        CircleRendererComponent(const CircleRendererComponent&) = default;
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
        std::shared_ptr<GltfModel::Model> model;

        MeshComponent() = default;
        MeshComponent(std::string_view meshname,std::string_view meshpath)
        {
            std::future<void> async_result = std::async(std::launch::async,[&](){
                name = meshname;
                path = meshpath;
                auto tStart = std::chrono::high_resolution_clock::now();
                model = std::make_shared<GltfModel::Model>();
                model->loadFromFile(path);
                auto tFileLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
                SG_CORE_INFO("Load Model Time:{} ms",tFileLoad);
            });
        
        }
        MeshComponent(const MeshComponent& other) :
        name(other.name),
        path(other.path),
        model(other.model)
        {}
    };

    //Material
    struct MaterialComponent{
        std::vector<ShaderMaterial> shaderMaterials{};
        VmaBuffer shaderMaterialBuffer;

        MaterialComponent() = default;
        MaterialComponent(std::vector<ShaderMaterial>& material){ shaderMaterials = material; }
        MaterialComponent(const MaterialComponent&) = default;
    };

    struct DirLightComponent{
        glm::vec3 Direction = {-0.2f,-1.0f,-0.3f};
        glm::vec3 Color{1.0f,1.0f,1.0f};
        float Intensity = 1.0f;//TODO: add intensity
        float m_cascade_frustum_near = 0.0f;
        bool castshadow = true;
        
        DirLightComponent() = default;
        DirLightComponent(const DirLightComponent&) = default;
    };
    
    struct SpoitLightComponent{
        glm::vec3 Color = glm::vec3(1.0f);
        glm::vec3 Direction = {-0.2f,-1.0f,-0.3f};
        float Intensity =  1.0f;
        float m_radius = 64.0f;
		float m_linear_attenuation = 0.14f;
		float m_quadratic_attenuation = 0.07f;
        float m_inner_cone_angle = 30.0f;
		float m_outer_cone_angle = 45.0f;
        bool castshadow = true;
        SpoitLightComponent() = default;
        SpoitLightComponent(const SpoitLightComponent&) = default;
    } ;

    struct PointLightComponent{
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity =  1.0f;
        bool castshadow = true;
        float m_radius = 64.0f;
		float m_linear_attenuation = 0.14f;
		float m_quadratic_attenuation = 0.07f;
        PointLightComponent() = default;
        PointLightComponent(const PointLightComponent&) = default;
    } ;


    struct SkyLightComponent{
        glm::vec3 Color= glm::vec3(1.0f);
        float Intensity = 0.5f;
        bool castshadow = true;
        
        std::string name = "null";
        std::string path;
        std::shared_ptr<SkyLightRenderData> skylight = nullptr;
        std::shared_ptr<TextureCube> textureCube = nullptr;
        bool Attach() {
            if (textureCube == nullptr)
                return false;
            skylight->CreateIBLTexture(textureCube);
            return true;
        }
        SkyLightComponent() = default;
        SkyLightComponent(const SkyLightComponent& other) :
        name(other.name),
        path(other.path),
        skylight(other.skylight),
        textureCube(other.textureCube)
        {}
    };

    struct AnimationComponent{
        int32_t animationIndex = 0;
        float animationTimer = 0.0f;
	    bool animate = false;
        std::vector<std::string> animationNames;
    };

};