#include "Texture2D.hpp"
#include "Core/Vulkan/Vulkantool.hpp"

namespace Sego{

    void Texture2D::loadFromMemory(){
        Vulkantool::createImageViewSampler(
        width_,height_,image_data_.data(),mip_levels_,layes_,format_,
        minfilter_,magfilter_,addressmode_u,image_view_sampler_);
    }





}