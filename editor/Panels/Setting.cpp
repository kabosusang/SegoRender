#include "Setting.hpp"
#include "Core/Vulkan/Vulkan_rhi.hpp"
#include "framework/Render/Render_data.hpp"
#include <imgui.h>

namespace Sego{

void Setting::OnImGuiRender(){
auto &Vctx = VulkanRhi::Instance();


if (!ImGui::Begin("Setting")){
        
        ImGui::End();
        return;
}
ImGui::SeparatorText("RenderDataSetting");
ImGui::DragFloat("exposure", &SceneRenderData.exposure,0.1f,0.0f, 10.0f);
ImGui::DragFloat("gamma", &SceneRenderData.gamma,0.1f,0.0f, 5.0f);
ImGui::End();


}





}