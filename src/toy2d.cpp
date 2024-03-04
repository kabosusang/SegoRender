#include "toy2d/toy2d.hpp"
#include "toy2d/context.hpp"

namespace toy2d{

void Init(const std::vector<const char*>& extensions,CreateSurfaceFunc func,int w,int h){
    Context::Init(extensions,func);
    Context::GetInstance().InitSwapChain(w,h);
}

void Quit(){
    Context::GetInstance().DestroySwapChain(); //destroy swapchain
    Context::Quit();
}

}

