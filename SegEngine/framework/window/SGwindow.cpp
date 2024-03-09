#include "SegEngine.h"


namespace Sego{

SGwindow* SGwindow::instance_ = nullptr;

void SGwindow::Init(int w,int h,int type)
{
    instance_ = new SGwindow(w,h,type);
}
SGwindow::SGwindow(int w,int h,int type)
{
    SDL_Init(type);
    window_ = SDL_CreateWindow("SgEngine",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          w, h,
                                          SDL_WINDOW_SHOWN|SDL_WINDOW_VULKAN|SDL_WINDOW_RESIZABLE);
    if (!window_) {
        SDL_Log("create window failed");
        exit(2);
    }
}

SGwindow::~SGwindow()
{
    SDL_DestroyWindow(window_);
    SDL_Quit();
}





}


