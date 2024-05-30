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

    std::shared_ptr<ImGuiImage> EditorUI::LoadFormFile(const std::string& path)
    {
		std::shared_ptr<ImGuiImage> image = std::make_shared<ImGuiImage>();
		image->image_view_sampler = Vulkantool::loadImageViewSampler(path);
		image->tex_id = ImGui_ImplVulkan_AddTexture(image->image_view_sampler.sampler, image->image_view_sampler.image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		image->is_owned = true;

        return image;
    }

	std::shared_ptr<ImGuiImage> EditorUI::LoadFormMemory(VmaImageViewSampler image_view_sampler)
    {
		std::shared_ptr<ImGuiImage> image = std::make_shared<ImGuiImage>();
		image->image_view_sampler = image_view_sampler;
		image->tex_id = ImGui_ImplVulkan_AddTexture(image->image_view_sampler.sampler, image->image_view_sampler.image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		image->is_owned = true;

        return image;
    }


}