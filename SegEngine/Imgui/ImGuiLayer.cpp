#include "pch.h"
#include "ImGuiLayer.hpp"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl2.h"
#include "SegEngine.h"
#include "SDL.h"

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
    void ImGuiLayer::OnUpdate(){
         // imgui new frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello, world!");
        ImGui::Text("Hello Wolrd");
        ImGui::End();

        //make imgui calculate internal draw structures
        ImGui::Render();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    void ImGuiLayer::OnEvent(Event& event){
         
    }




}
