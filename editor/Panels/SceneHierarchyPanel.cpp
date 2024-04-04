#include "SceneHierarchyPanel.hpp"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Scene/Component.hpp"

namespace Sego{

SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene>& scene){
    SetContext(scene);
}

void SceneHierarchyPanel::SetContext(const std::shared_ptr<Scene>& scene){
    m_Context = scene;
}

void SceneHierarchyPanel::OnImGuiRender(){
    ImGui::Begin("Scene Hierarchy");

    auto view = m_Context->m_Registry.view<TagComponent>();
    for (auto entityID : view) {
        Entity entity = {entityID, m_Context.get()};
        DrawEnityNode(entity);
    }
    if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
        m_SelectionContext = {};
    
    ImGui::End();

    ImGui::Begin("Inspector");
    if(m_SelectionContext){
        DrawComponents(m_SelectionContext);
    }
    ImGui::End();

}

void SceneHierarchyPanel::DrawEnityNode(Entity entity){

    auto& tag = entity.GetComponent<TagComponent>().Tag;
    
    ImGuiTreeNodeFlags flags =((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity,flags,tag.c_str());
    if(ImGui::IsItemClicked()){
        m_SelectionContext = entity;
    }

    if(opened){
        ImGuiTreeNodeFlags flags =((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity,flags,tag.c_str());
        if(opened){
                ImGui::TreePop(); 
        }
        ImGui::TreePop();
    }
}

void SceneHierarchyPanel::DrawComponents(Entity entity){
    if(entity.HasComponent<TagComponent>()){
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        char buffer[256];
        memset(buffer,0,sizeof(buffer));
        strcpy_s(buffer,sizeof(buffer),tag.c_str());
        if(ImGui::InputText("Tag",buffer,sizeof(buffer))){
            tag = std::string(buffer);
        }


    }

    if(entity.HasComponent<TransformComponent>()){
        if(ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(),ImGuiTreeNodeFlags_DefaultOpen,"Transform")){
            
            auto& transform =  entity.GetComponent<TransformComponent>().Transform;
            ImGui::DragFloat3("Position",glm::value_ptr(transform[3]),0.25f);
            ImGui::TreePop();
        }
        
    }

    if(entity.HasComponent<CameraComponent>()){
        if(ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(),ImGuiTreeNodeFlags_DefaultOpen,"Camera")){
            
            auto& cameraComponent =  entity.GetComponent<CameraComponent>();
            auto& camera = cameraComponent.Camera;
            
            const char* projectionTypeStirngs[] = {"Perspective","Orthographic"};
            const char* currentProjectionTypeString = projectionTypeStirngs[(int)camera.GetProjectionType()];

            if(ImGui::BeginCombo("Projection",currentProjectionTypeString)){
                
                for(int i = 0; i < 2; i++){
                    bool isSelected = currentProjectionTypeString == projectionTypeStirngs[i];
                    if(ImGui::Selectable(projectionTypeStirngs[i],isSelected)){
                        currentProjectionTypeString == projectionTypeStirngs[i];
                        camera.SetProjectionType((SceneCamera::ProjectionType)i);
                    }

                    if(isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if(camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective){
               
            }

            if(camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic){
                float orthSize =  camera.GetOrthographicSize();
                if(ImGui::DragFloat("Size",&orthSize))
                    camera.SetOrthographicSize(orthSize);
            
                float orthoNear = camera.GetOrthographicNearClip();
                if(ImGui::DragFloat("Near Clip",&orthoNear))
                    camera.SetOrthographicNearClip(orthoNear);
                
                float orthoFar = camera.GetOrthographicFarClip();
                if(ImGui::DragFloat("Far Clip",&orthoFar))
                    camera.SetOrthographicFarClip(orthoFar);

            }
            ImGui::TreePop();
        }
    }
}






}