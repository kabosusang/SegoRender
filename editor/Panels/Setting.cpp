#include "Setting.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include <imgui.h>

float BiasConstant = 1.25f;
float BiasSlope = 7.5f;
namespace Sego{

void Setting::OnImGuiRender(){
auto &Vctx = VulkanRhi::Instance();


if (!ImGui::Begin("Setting")){
        
        ImGui::End();
        return;
}

ImGui::DragFloat("BiasConstant", &BiasConstant,0.1f,0.0f, 100.0f);
ImGui::DragFloat("BiasSlope", &BiasSlope,0.1f,0.0f, 100.0f);
Vctx.SetDepthBias(BiasConstant,BiasSlope);

ImGui::End();


}





}