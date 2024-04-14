#pragma once

#include <filesystem>
#include <resource/asset/Texture2D.hpp>
#include "../editor/EditorUI.hpp"

namespace Sego{

    class ContentBrowsPanel{
    public:
        ContentBrowsPanel();

        void OnImGuiRender();
    private:
        std::filesystem::path m_CurrentDirectory;

        std::shared_ptr<ImGuiImage> m_DirectoryIcon;
        std::shared_ptr<ImGuiImage> m_FileIcon;
    };





}