#include "toy2d.hpp"
#include "SDL.h"
#include "SDL_vulkan.h"
#include <SDL_video.h>
#include <iostream>

namespace Sego{

    class SGwindow
    {
    public:
        ~SGwindow();
        static void Init(int w,int h,int type = SDL_INIT_EVERYTHING);

        static SGwindow* Instance(){
            return instance_;
        }

        inline SDL_Window* getWindow(){return window_;}
        inline void WindowResizeStatue(bool status){ WindowResized = status;}
        inline bool WindowResizedStatue(){return WindowResized;}
    private:
        //instance
        static SGwindow* instance_;
        SGwindow(int w,int h,int type = SDL_INIT_EVERYTHING);

        //w,h
        int height;
        int width;
        //SDL window
        SDL_Window* window_;
        //bool
        bool WindowResized = false;
    };










}