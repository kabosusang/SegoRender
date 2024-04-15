#include "pch.h"
#include "ContentBrowsPanel.hpp"
#include <imgui.h>


namespace Sego{
    //once we have projects,change this to the project directory
    extern const std::filesystem::path s_AssetPath = "resources";

ContentBrowsPanel::ContentBrowsPanel():m_CurrentDirectory(s_AssetPath){
    m_DirectoryIcon = EditorUI::LoadFormFile("resources/Settings/icons/ContentBrowser/DirectoryIcon.png");
    m_FileIcon = EditorUI::LoadFormFile("resources/Settings/icons/ContentBrowser/FileIcon.png");

}
//resources/Settings/icons/ContentBrowser
void ContentBrowsPanel::OnImGuiRender(){
    ImGui::Begin("Content Browser");
    if (m_CurrentDirectory != std::filesystem::path(s_AssetPath)){
        if (ImGui::Button("<-")){
            m_CurrentDirectory = m_CurrentDirectory.parent_path();
        }
    }

    static float padding = 16.0f;
    static float thumbnailSize = 128;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    for (auto& directoryEntry :  std::filesystem::directory_iterator(m_CurrentDirectory)){
        
        const auto& path = directoryEntry.path();
        auto relativePath = std::filesystem::relative(path,s_AssetPath);
        std::string filenameString = relativePath.filename().string();
        //id
        ImGui::PushID(filenameString.c_str());

        std::shared_ptr<ImGuiImage> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
        ImGui::ImageButton((ImTextureID)icon->tex_id, {thumbnailSize,thumbnailSize},{1,0},{0,1});
        
        if (ImGui::BeginDragDropSource()){
            const wchar_t* itemPath = relativePath.c_str();
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath)+1) * sizeof(wchar_t), ImGuiCond_Once);
            ImGui::EndDragDropSource();
        }
        
        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)){
            
            if (directoryEntry.is_directory()){
                m_CurrentDirectory /= path.filename();
            }
        }
        ImGui::TextWrapped(filenameString.c_str());
        ImGui::NextColumn();

        ImGui::PopID();
    }
    ImGui::Columns(1);

    ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
    ImGui::SliderFloat("Padding", &padding, 0, 32);

    ImGui::End();

}







}


