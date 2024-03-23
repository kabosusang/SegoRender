#include "pch.h"
#include "WindowInput.hpp"

#include <SDL.h>

namespace Sego{
Input* Input::instance_ = new WindowsInput();

bool WindowsInput::IsKeyPressedImpl(int keycode)
{  
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[keycode])
        return true;

    return false;
}

bool WindowsInput::IsMouseButtonPressedImpl(int button)
{
    const Uint32 state = SDL_GetMouseState(NULL, NULL);
    if (state & SDL_BUTTON(button))
        return true;

    return false;
}

float WindowsInput::GetMouseXImpl()
{
    auto v = GetMousePosImpl();
    return std::get<0>(v);
}

float WindowsInput::GetMouseYImpl()
{
    auto v = GetMousePosImpl();
    return std::get<1>(v);
}

std::pair<float, float> WindowsInput::GetMousePosImpl()
{
    int x,y;
    int mouseX, mouseY;  // 鼠标相对窗口的坐标

    //auto window = SDL_GetMouseFocus();
    SDL_GetMouseState(&mouseX, &mouseY);
    return {(float)mouseX,(float)mouseY};
}

std::pair<uint32_t, uint32_t> WindowsInput::GetWindowSizeImpl()
{
    int w, h;
    SDL_GetWindowSize(SDL_GetWindowFromID(1), &w, &h);
    return {w, h};
}



}

