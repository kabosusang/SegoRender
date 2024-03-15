#include "base.hpp"
#include "SDL.h"

namespace Sego{
    void ImGuiBase::ImGuiRenderBase(){
        // imgui new frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiBase::ImGuiRenderEndBase(){
        //make imgui calculate internal draw structures
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
    }

    void ImGuiBase::Render(){
        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello, world!");
        ImGui::Text("Hello 世界");
        ImGui::End();
        
    }

} // namespace Sego





