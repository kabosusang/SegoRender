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
    ImGui::Render();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

void ImGuiLayer::OnImGuiRender(){
    ImGui::ShowDemoWindow();
    ImGui::Begin("Hello, world!");
    ImGui::Text("Hello Wolrd");
    ImGui::End();
}

void ImGuiLayer::OnEvent(Event& event){
    
}






}
