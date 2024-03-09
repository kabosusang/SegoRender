#pragma once 
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl2.h>

namespace Sego{

class ImGuiBase{
    public:
    static void Render();
    static void ImGuiRenderBase();
    static void ImGuiRenderEndBase();



};
}