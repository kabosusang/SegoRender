#include "SceneHierarchyPanel.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Scene/Component.hpp"
#include "resource/asset/Import/gltf_import.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"

#include <future>

namespace Sego{

extern const std::filesystem::path s_AssetPath;

SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene>& scene){
    SetContext(scene);
}

void SceneHierarchyPanel::SetContext(const std::shared_ptr<Scene>& scene){
    m_Context = scene;
    m_SelectionContext = {};
}

void SceneHierarchyPanel::OnImGuiRender(){
    ImGui::Begin("Scene Hierarchy");

    if (m_Context){
        auto view = m_Context->m_Registry.view<TagComponent>();
        for (auto entityID : view) {
            Entity entity = {entityID, m_Context.get()};
            DrawEnityNode(entity);
        }
        if(ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_SelectionContext = {};
        
        //Right click on the window
        if (ImGui::BeginPopupContextWindow(0,ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
                m_Context->CreateEntity("Empty Entity");
            ImGui::SeparatorText("Object");
            if (ImGui::BeginMenu("Create")){
                if (ImGui::BeginMenu("2D")){
                    if (ImGui::MenuItem("Sprite")){
                        Entity curent = m_Context->CreateEntity("Sprite");
                        curent.AddComponent<SpriteRendererComponent>();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("3D")){
                    if (ImGui::MenuItem("Cube")){
                        Entity curent = m_Context->CreateEntity("Cube");
                        curent.AddComponent<MeshComponent>("Cube","resources/Settings/SceneObject/Cube.glb");
                    }

                    if (ImGui::MenuItem("Plate")){
                        Entity curent = m_Context->CreateEntity("Plane");
                        curent.AddComponent<MeshComponent>("Plane","resources/Settings/SceneObject/Plane.glb");
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            } 
            ImGui::SeparatorText("Light");
            if (ImGui::BeginMenu("Light")){
                if (ImGui::MenuItem("Directional Light")){
                    Entity curent = m_Context->CreateEntity("Directional Light");
                    curent.AddComponent<DirLightComponent>();
                }
                
                if (ImGui::MenuItem("Sky Light")){
                    Entity curent = m_Context->CreateEntity("Sky Light");
                    curent.AddComponent<SkyLightComponent>();
                }

                if (ImGui::MenuItem("Poit Light")){
                    Entity curent = m_Context->CreateEntity("Poit Light");
                    curent.AddComponent<PointLightComponent>();
                }

                 if (ImGui::MenuItem("Spot Light")){
                    Entity curent = m_Context->CreateEntity("Spot Light");
                    curent.AddComponent<SpoitLightComponent>();
                }

                ImGui::EndMenu();
            }
            ImGui::EndPopup();    
        }
    }
    ImGui::End();

    ImGui::Begin("Inspector");
    if(m_SelectionContext){
        DrawComponents(m_SelectionContext);
    }
    ImGui::End();
}

void SceneHierarchyPanel::SetSelectedEntity(Entity entity){
    m_SelectionContext = entity;
}

void SceneHierarchyPanel::AttachMaterial(Entity entity)
{
    std::vector<ShaderMaterial> shaderMaterials{};
    auto& mesh = entity.GetComponent<MeshComponent>();
    for (auto& material : mesh.model->materials) {
        ShaderMaterial shaderMaterial{};
        shaderMaterial.emissiveFactor = material.emissiveFactor;
        // To save space, availabilty and texture coordinate set are combined
        // -1 = texture not used for this material, >= 0 texture used and index of texture coordinate set
        shaderMaterial.colorTextureSet = material.baseColorTexture != nullptr ? material.texCoordSets.baseColor : -1;                  
        shaderMaterial.normalTextureSet = material.normalTexture != nullptr ? material.texCoordSets.normal : -1;                   
        shaderMaterial.occlusionTextureSet = material.occlusionTexture != nullptr ? material.texCoordSets.occlusion : -1;
        shaderMaterial.emissiveTextureSet = material.emissiveTexture != nullptr ? material.texCoordSets.emissive : -1;
        shaderMaterial.alphaMask = static_cast<float>(material.alphaMode == GltfModel::PBRMaterial::ALPHAMODE_MASK);
        shaderMaterial.alphaMaskCutoff = material.alphaCutoff;
        shaderMaterial.emissiveStrength = material.emissiveStrength;

        // TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present
        if (material.pbrWorkflows.metallicRoughness) {
            // Metallic roughness workflow
            shaderMaterial.workflow = static_cast<float>(PBR_WORKFLOW_METALLIC_ROUGHNESS);
            shaderMaterial.baseColorFactor = material.baseColorFactor;
            shaderMaterial.metallicFactor = material.metallicFactor;
            shaderMaterial.roughnessFactor = material.roughnessFactor;
            shaderMaterial.PhysicalDescriptorTextureSet = material.metallicRoughnessTexture != nullptr ? material.texCoordSets.metallicRoughness : -1;    
            shaderMaterial.colorTextureSet = material.baseColorTexture != nullptr ? material.texCoordSets.baseColor : -1;
            
        }
        if (material.pbrWorkflows.specularGlossiness) {
            // Specular glossiness workflow
            shaderMaterial.workflow = static_cast<float>(PBR_WORKFLOW_SPECULAR_GLOSINESS);
            shaderMaterial.PhysicalDescriptorTextureSet = material.extension.specularGlossinessTexture != nullptr ? material.texCoordSets.specularGlossiness : -1;
            shaderMaterial.colorTextureSet = material.extension.diffuseTexture != nullptr ? material.texCoordSets.baseColor : -1;   
            shaderMaterial.diffuseFactor = material.extension.diffuseFactor;
            shaderMaterial.specularFactor = glm::vec4(material.extension.specularFactor, 1.0f);
        }
        shaderMaterials.push_back(shaderMaterial);
    }
    
    entity.AddComponent<MaterialComponent>(shaderMaterials);
    auto& matComt = entity.GetComponent<MaterialComponent>();
    if (!shaderMaterials.empty()){
        auto maxFlightCount_ = VulkanRhi::Instance().getMaxFlightCount(); 
        vk::DeviceSize bufferSize = sizeof(ShaderMaterial) * shaderMaterials.size();
        Vulkantool::createMaterialBuffer(bufferSize,shaderMaterials.data(), matComt.shaderMaterialBuffer);
    }
}

void SceneHierarchyPanel::DrawEnityNode(Entity entity)
{

    auto& tag = entity.GetComponent<TagComponent>().Tag;
    
    ImGuiTreeNodeFlags flags =((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity,flags,tag.c_str());
    if(ImGui::IsItemClicked()){
        m_SelectionContext = entity;
    }

    bool entityDeleted = false;
    if (ImGui::BeginPopupContextItem()){
        if (ImGui::MenuItem("Delete Entity"))
            entityDeleted = true;
        ImGui::EndPopup();
    }

    if(opened){
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |ImGuiTreeNodeFlags_SpanAvailWidth ;
        bool opened = ImGui::TreeNodeEx((void*)99891823,flags,tag.c_str());
        if(opened){
                ImGui::TreePop(); 
        }
        ImGui::TreePop();
    }

    if(entityDeleted){
        m_Context->DestroyEntity(entity);
        if(m_SelectionContext == entity)
            m_SelectionContext = {};
    }
}

static void DrawVec3Control(const std::string& label,glm::vec3& valuesm,
float resetValue = 0.0f,float columWidth = 100.0f){
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

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
    ImGui::PushFont(boldFont);
    if(ImGui::Button("X",buttonSize))
        valuesm.x = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X",&valuesm.x,0.1f,0.0f,0.0f,"%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    
    //button Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.7f,0.2f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f,0.8f,0.3f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f,0.6f,0.1f,1.0f));
    ImGui::PushFont(boldFont);
    if(ImGui::Button("Y",buttonSize))
        valuesm.y = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y",&valuesm.y,0.1f,0.0f,0.0f,"%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    //button Z
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.1f,0.8f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f,0.2f,0.9f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f,0.1f,0.7f,1.0f));
    ImGui::PushFont(boldFont);
    if(ImGui::Button("Z",buttonSize))
        valuesm.z = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z",&valuesm.z,0.1f,0.0f,0.0f,"%.2f");
    ImGui::PopItemWidth();


    ImGui::PopStyleVar();
    ImGui::Columns(1);

    ImGui::PopID();
}

template<typename T, typename UIFunction>
static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
{
    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

    if (entity.HasComponent<T>())
    {
        auto& component = entity.GetComponent<T>();
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
        ImGui::PopStyleVar();
        
        ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
        if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
        {
            ImGui::OpenPopup("ComponentSettings");
        }

        bool removeComponent = false;
        if (ImGui::BeginPopup("ComponentSettings"))
        {
            if (ImGui::MenuItem("Remove component"))
                removeComponent = true;

            ImGui::EndPopup();
        }

        if (open)
        {
            uiFunction(component);
            ImGui::TreePop();
        }

        if (removeComponent)
            entity.RemoveComponent<T>();
    }
}

void SceneHierarchyPanel::DrawComponents(Entity entity)
{
    if (entity.HasComponent<TagComponent>())
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy_s(buffer, sizeof(buffer), tag.c_str());
        if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
        {
            tag = std::string(buffer);
        }
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);

    if (ImGui::Button("Add Component"))
        ImGui::OpenPopup("AddComponent");

    if (ImGui::BeginPopup("AddComponent"))
    {
        if (!m_SelectionContext.HasComponent<CameraComponent>()){
            if (ImGui::MenuItem("Camera"))
            {
                m_SelectionContext.AddComponent<CameraComponent>();
                ImGui::CloseCurrentPopup();
            }
        }
        
        if (!m_SelectionContext.HasComponent<SpriteRendererComponent>() && !m_SelectionContext.HasComponent<MeshComponent>()){
            if (ImGui::MenuItem("Sprite Renderer"))
            {
                m_SelectionContext.AddComponent<SpriteRendererComponent>();
                ImGui::CloseCurrentPopup();
            }
        }
        /*
          if (!m_SelectionContext.HasComponent<CircleRendererComponent>() && !m_SelectionContext.HasComponent<MeshComponent>()){
            if (ImGui::MenuItem("Circle Renderer"))
            {
                m_SelectionContext.AddComponent<CircleRendererComponent>();
                ImGui::CloseCurrentPopup();
            }
        }*/


        if (!m_SelectionContext.HasComponent<Rigidbody2DComponent>()){
            if (ImGui::MenuItem("Rigidbody 2D"))
            {
                m_SelectionContext.AddComponent<Rigidbody2DComponent>();
                ImGui::CloseCurrentPopup();
            }
        }

        if (!m_SelectionContext.HasComponent<BoxCollider2Domponent>()){
            if (ImGui::MenuItem("Box Collider 2D"))
            {
                m_SelectionContext.AddComponent<BoxCollider2Domponent>();
                ImGui::CloseCurrentPopup();
            }
        }

        if (!m_SelectionContext.HasComponent<MeshComponent>() && !m_SelectionContext.HasComponent<SpriteRendererComponent>()){
            if (ImGui::MenuItem("MeshComponent"))
            {
                m_SelectionContext.AddComponent<MeshComponent>();
                ImGui::CloseCurrentPopup();
            }
        }

        if (!m_SelectionContext.HasComponent<MaterialComponent>() && !m_SelectionContext.HasComponent<SpriteRendererComponent>() && m_SelectionContext.HasComponent<MeshComponent>()){
        
            if (ImGui::MenuItem("Material"))
            {
                AttachMaterial(m_SelectionContext); //Material Attach
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::PopItemWidth();

    DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
    {
        DrawVec3Control("Translation", component.Translation);
        glm::vec3 rotation = glm::degrees(component.Rotation);
        DrawVec3Control("Rotation", rotation);
        component.Rotation = glm::radians(rotation);
        DrawVec3Control("Scale", component.Scale, 1.0f);
    });

    DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
    {
        auto& camera = component.Camera;

        ImGui::Checkbox("Primary", &component.Primary);

        const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
        const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
        if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
        {
            for (int i = 0; i < 2; i++)
            {
                bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                {
                    currentProjectionTypeString = projectionTypeStrings[i];
                    camera.SetProjectionType((SceneCamera::ProjectionType)i);
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
        {
            float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
            if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
                camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

            float perspectiveNear = camera.GetPerspectiveNearClip();
            if (ImGui::DragFloat("Near", &perspectiveNear))
                camera.SetPerspectiveNearClip(perspectiveNear);

            float perspectiveFar = camera.GetPerspectiveFarClip();
            if (ImGui::DragFloat("Far", &perspectiveFar))
                camera.SetPerspectiveFarClip(perspectiveFar);
        }

        if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
        {
            float orthoSize = camera.GetOrthographicSize();
            if (ImGui::DragFloat("Size", &orthoSize))
                camera.SetOrthographicSize(orthoSize);

            float orthoNear = camera.GetOrthographicNearClip();
            if (ImGui::DragFloat("Near", &orthoNear))
                camera.SetOrthographicNearClip(orthoNear);

            float orthoFar = camera.GetOrthographicFarClip();
            if (ImGui::DragFloat("Far", &orthoFar))
                camera.SetOrthographicFarClip(orthoFar);

            ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
        }
    });

    DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
    {
        ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
        //Texture
        //ImGui::DragFloat("Tiling Factor", &component.TilingFactor,0.1f,0.0f,100.0f);
        ImGui::Button("Texture",ImVec2(100.0f,0.0f));
        if (ImGui::BeginDragDropTarget()){
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")){
			const wchar_t* path = (const wchar_t*)payload->Data;
			std::filesystem::path texturePath = std::filesystem::path(s_AssetPath)/path;
            if (texturePath.extension() == ".png" || texturePath.extension() == ".jpg" || texturePath.extension() == ".jpeg" || texturePath.extension() == ".bmp")
            {
                component.Texture = Texture2D::Create(texturePath.string());
            }else{
                SG_CORE_ERROR("Texture format not supported");
            }        
		}
	
		ImGui::EndDragDropTarget();
	}

    });
    /*
    DrawComponent<CircleRendererComponent>("Sprite Renderer", entity, [](auto& component)
    {
        ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
        ImGui::DragFloat("Thickness", &component.Thickness,0.025f,0.0f, 1.0f);
        ImGui::DragFloat("Fade", &component.Fade,0.00025f,0.0f, 1.0f);
	
    });*/

    DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
    {
        const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
        const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];

        if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
        {
            for (int i = 0; i < 3; i++)
            {
                bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                {
                    currentBodyTypeString = bodyTypeStrings[i];
                    component.Type = (Rigidbody2DComponent::BodyType)i;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
    });

    DrawComponent<BoxCollider2Domponent>("Box Collider 2D", entity, [](auto& component)
    {
        ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
        ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
        ImGui::DragFloat("Density", &component.Density,0.01f,0.0f, 0.1f);
        ImGui::DragFloat("Friction", &component.Friction, 0.01f,0.0f, 0.1f);
        ImGui::DragFloat("Restitution", &component.Restitution, 0.01f,0.0f, 1.0f);
        ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f,0.0f);
    });

    DrawComponent<MeshComponent>("MeshComponent", entity, [&](auto& component)
    {
        ImGui::Text("Mesh");
        ImGui::SameLine();
        ImGui::Button(component.name.c_str(),ImVec2(300.0f,0.0f));
        if (ImGui::BeginDragDropTarget()){
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")){
			const wchar_t* path = (const wchar_t*)payload->Data;
			std::filesystem::path modelPath = std::filesystem::path(s_AssetPath)/path;
            if (modelPath.extension() == ".gltf" || modelPath.extension() == ".glb")
            {
                std::future<void> async_result = std::async(std::launch::async,[&](){
                    component.name = modelPath.filename().replace_extension().string();
                    component.path = modelPath.string();
                    auto tStart = std::chrono::high_resolution_clock::now();
                    component.model = std::make_shared<GltfModel::Model>();
                    component.model->loadFromFile(component.path);
                    auto tFileLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
                    SG_CORE_INFO("Load Model Time:{} ms",tFileLoad);
                    if (entity.HasComponent<MaterialComponent>()){
                        entity.RemoveComponent<MaterialComponent>();
                        AttachMaterial(entity);
                    }
                });
            }else{
                SG_CORE_ERROR("Model format not supported");
            }
        }
		ImGui::EndDragDropTarget();
    }

    });

    //Material
    DrawComponent<MaterialComponent>("Material", entity, [](auto& component){
       
    });

    //Light Component
    DrawComponent<SkyLightComponent>("SkyLightComponent", entity, [](auto& component){
        ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
        ImGui::Checkbox("Shadow",&component.castshadow);
        ImGui::Text("IBL");
        ImGui::SameLine();
        ImGui::Button(component.name.c_str(),ImVec2(300.0f,0.0f));
        if (ImGui::BeginDragDropTarget()){
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")){
                const wchar_t* path = (const wchar_t*)payload->Data;
                std::filesystem::path texturePath = std::filesystem::path(s_AssetPath)/path;
                if (texturePath.extension() == ".ktx")
                {
                    component.name = texturePath.filename().replace_extension().string();
                    component.path = texturePath.string();
                    component.textureCube = TextureCube::Create(component.path);
                    component.skylight = std::make_shared<SkyLightRenderData>();
                    component.Attach();
                }else{
                    SG_CORE_ERROR("Texture format not supported");
                }        
            }
        }

    });

    DrawComponent<DirLightComponent>("DirLight", entity, [](auto& component){
        DrawVec3Control("Direction", component.Direction);
        ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
        ImGui::DragFloat("Intensity", &component.Intensity,0.1f,0.0f, 10.0f);
        ImGui::DragFloat("Near Planes",&component.m_cascade_frustum_near,0.1f,0.0f, 10.0f);
        ImGui::Checkbox("Shadow",&component.castshadow);
    });

    
    DrawComponent<PointLightComponent>("PointLight", entity, [](auto& component){
        ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
        ImGui::DragFloat("Intensity", &component.Intensity,0.1f,0.0f, 10.0f);
        ImGui::DragFloat("Radius", &component.m_radius,0.01f,0.0f, 10.0f);
        ImGui::DragFloat("Linear", &component.m_linear_attenuation,0.01f,0.0f, 1.0f);
        ImGui::DragFloat("Quadratic", &component.m_quadratic_attenuation,0.01f,0.0f, 1.0f);
        ImGui::Checkbox("Shadow",&component.castshadow);
    });

     DrawComponent<SpoitLightComponent>("SpotLight", entity, [](auto& component){
        DrawVec3Control("Direction", component.Direction);
        ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
        ImGui::DragFloat("Intensity", &component.Intensity,0.1f,0.0f, 10.0f);
        ImGui::DragFloat("Radius", &component.m_radius,1.0f,0.0f, 1000.0f);
        ImGui::DragFloat("Linear", &component.m_linear_attenuation,0.01f,0.0f, 1.0f);
        ImGui::DragFloat("Quadratic", &component.m_quadratic_attenuation,0.01f,0.0f, 1.0f);
        ImGui::SliderFloat("Inner Cone Angle", &component.m_inner_cone_angle, 0.0f, 90.0f);
        ImGui::SliderFloat("Outer Cone Angle", &component.m_outer_cone_angle, 0.0f, 90.0f);
        ImGui::Checkbox("Shadow",&component.castshadow);

    });


}


}