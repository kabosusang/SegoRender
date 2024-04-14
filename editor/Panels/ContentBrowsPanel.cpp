#include "pch.h"
#include "ContentBrowsPanel.hpp"
#include <imgui.h>

namespace Sego{
    //once we have projects,change this to the project directory
    constexpr char* s_AsstsDirectory = "resources/assets";

void ContentBrowsPanel::OnImGuiRender(){
    ImGui::Begin("Content Browser");


    for (auto& p :  std::filesystem::directory_iterator(s_AsstsDirectory)){
        std::string path = p.path().string();
        ImGui::Text("%s", path.c_str());
    }

    ImGui::End();


}

}


