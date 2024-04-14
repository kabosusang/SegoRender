#pragma once

#include <filesystem>

namespace Sego{

    class ContentBrowsPanel{
    public:
        ContentBrowsPanel() = default;

        void OnImGuiRender();
    private:
        std::filesystem::path m_CurrentDirectory = "resources";

    };





}