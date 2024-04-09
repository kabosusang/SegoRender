#include "pch.h"
#include "Input.hpp"
#include <SDL.h>

namespace Sego{

bool Input::ISKeyPressed(int keycode)
{  
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[keycode])
        return true;

    return false;
}

bool Input::IsMouseButtonPressed(int button)
{
    const Uint32 state = SDL_GetMouseState(NULL, NULL);
    if (state & SDL_BUTTON(button))
        return true;
    
    return false;
}

float Input::GetMouseX()
{
    auto v = GetMousePos();
    return std::get<0>(v);
}

float Input::GetMouseY()
{
    auto v = GetMousePos();
    return std::get<1>(v);
}

std::pair<float, float> Input::GetMousePos()
{
    int x,y;
    int mouseX, mouseY;  // 鼠标相对窗口的坐标
    //auto window = SDL_GetMouseFocus();
    SDL_GetMouseState(&mouseX, &mouseY);
    return {(float)mouseX,(float)mouseY};
}

std::pair<uint32_t, uint32_t> Input::GetWindowSize()
{
    int w, h;
    SDL_GetWindowSize(SDL_GetWindowFromID(1), &w, &h);
    return {w, h};
}



}

