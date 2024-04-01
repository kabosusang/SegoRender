#include "EditorUI.hpp"
#include <imgui_impl_vulkan.h>

namespace Sego{
    void ImGuiImage::destory(){
        if (is_owned)
		{
			image_view_sampler.destroy();
		}
		
		ImGui_ImplVulkan_RemoveTexture(tex_id);
    }








}