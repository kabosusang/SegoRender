#pragma once

#include "context.hpp"
#include "render_process.hpp"
#include "renderer.hpp"
#include "descriptor_manager.hpp"
#include <memory>

namespace Sego{

void Init(std::vector<const char*>& extensions, Context::GetSurfaceCallback, int windowWidth, int windowHeight);
void Quit();
Texture* LoadTexture(const std::string& filename);
void DestroyTexture(Texture*);
Renderer* GetRenderer();

}
