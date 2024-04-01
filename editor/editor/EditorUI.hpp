#pragma once
#include "Core/Vulkan/Vulkantool.hpp"

namespace Sego{

    struct ImGuiImage{
        VmaImageViewSampler image_view_sampler;
        vk::DescriptorSet tex_id;

        bool is_owned = true;
        void destory();
    };




}