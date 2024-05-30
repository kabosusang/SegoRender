#pragma once
#include "Core/Vulkan/Vulkantool.hpp"

namespace Sego{

    struct ImGuiImage{
        VmaImageViewSampler image_view_sampler;
        vk::DescriptorSet tex_id;

        bool is_owned = true;
        void destory();
    };

    class EditorUI{
    public:
        static std::shared_ptr<ImGuiImage> LoadFormFile(const std::string& path);
        static std::shared_ptr<ImGuiImage> LoadFormMemory(VmaImageViewSampler image_view_sampler);
    };




}