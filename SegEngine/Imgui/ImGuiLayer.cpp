#include "pch.h"
#include "ImGuiLayer.hpp"
#include "SegEngine.h"

#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl2.h>

namespace Sego{
ImGuiLayer::ImGuiLayer()
    : Layer("ImGuiLayer"){
}
ImGuiLayer::~ImGuiLayer(){
}

void ImGuiLayer::OnAttach(){
    
}
void ImGuiLayer::OnDetach(){
    
    
}

void ImGuiLayer::Begin(){
    // imgui new frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

}
void ImGuiLayer::End(){
      // 渲染 ImGui 绘制的界面
    ImGui::Render();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

void ImGuiLayer::OnImGuiRender(){
    
}

void ImGuiLayer::OnEvent(Event& event){
    if (m_BlockEvents){
        ImGuiIO& io = ImGui::GetIO();
        event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        event.Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
    }


}






}
