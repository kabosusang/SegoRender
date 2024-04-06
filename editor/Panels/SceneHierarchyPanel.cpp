#include "SceneHierarchyPanel.hpp"
#include <imgui.h>
#include <imgui_internal.h>
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

static void DrawVec3Control(const std::string& label,glm::vec3& valuesm,
float resetValue = 0.0f,float columWidth = 100.0f){
    ImGui::PushID(label.c_str()); //Make sure the ID is unique

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0,columWidth);
    ImGui::Text(label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3,ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0,0});

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

    //button X
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f,0.1f,0.15f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f,0.2f,0.2f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f,0.1f,0.1f,1.0f));
    if(ImGui::Button("X",buttonSize))
        valuesm.x = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X",&valuesm.x,0.1f,0.0f,0.0f,"%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    
    //button Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.7f,0.2f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f,0.8f,0.3f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f,0.6f,0.1f,1.0f));
    if(ImGui::Button("Y",buttonSize))
        valuesm.y = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y",&valuesm.y,0.1f,0.0f,0.0f,"%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    //button Z
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.1f,0.8f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f,0.2f,0.9f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f,0.1f,0.7f,1.0f));
    if(ImGui::Button("Z",buttonSize))
        valuesm.z = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z",&valuesm.z,0.1f,0.0f,0.0f,"%.2f");
    ImGui::PopItemWidth();


    ImGui::PopStyleVar();
    ImGui::Columns(1);

    ImGui::PopID();
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
            
            auto& tc =  entity.GetComponent<TransformComponent>();
            DrawVec3Control("Translation",tc.Translation);
            glm::vec3 rotation = glm::degrees(tc.Rotation);
            DrawVec3Control("Rotation" ,rotation);
            tc.Rotation = glm::radians(rotation);
            DrawVec3Control("Scale",tc.Scale,1.0f);

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
                
                float verticalFov =  glm::degrees(camera.GerPerspectiveVerticalFOV());
                if(ImGui::DragFloat("FOV",&verticalFov))
                    camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));
            
                float Near = camera.GerPerspectiveNearClip();
                if(ImGui::DragFloat("Near Clip",&Near))
                    camera.SetPerspectiveNearClip(Near);
                
                float Far = camera.GerPerspectiveFarClip();
                if(ImGui::DragFloat("Far Clip",&Far))
                    camera.SetPerspectiveFarClip(Far);
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

    if(entity.HasComponent<SpriteRendererComponent>()){
        if(ImGui::TreeNodeEx((void*)typeid(SpriteRendererComponent).hash_code(),ImGuiTreeNodeFlags_DefaultOpen,"Sprite Renderer")){
            
            auto& src = entity.GetComponent<SpriteRendererComponent>();
            ImGui::ColorEdit4("Color",glm::value_ptr(src.Color));
            
            ImGui::TreePop();
        }
        
    }



}

   



}