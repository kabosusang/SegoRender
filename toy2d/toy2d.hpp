#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "vulkan/vulkan.hpp"

namespace toy2d{

void Init(const std::vector<const char*>& extensions,CreateSurfaceFunc func,int w,int h);
void Quit();

}