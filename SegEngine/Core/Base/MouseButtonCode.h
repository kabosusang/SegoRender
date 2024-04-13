#pragma once

//IN OUR ENGINE  (ChangeForm MouseEvent.h)
enum Mouse{
    ButtonLeft = 0,
    ButtonRight = 1,
    ButtonMiddle = 2
};

//emmmmmmmmmmm I think this is a bad idea
// SDL_mouse.h 
enum InputMouse{
    Left = 1,
    Middle = 2,
    Right = 3
};

//Form SDL_mouse.h
#define SG_MOUSE_BUTTON(X)       (1 << ((X)-1))
#define SG_MOUSE_BUTTON_LEFT     1
#define SG_MOUSE_BUTTON_MIDDLE   2
#define SG_MOUSE_BUTTON_RIGHT    3
#define SG_MOUSE_BUTTON_X1       4
#define SG_MOUSE_BUTTON_X2       5
#define SG_MOUSE_BUTTON_LMASK    SG_MOUSE_BUTTON(SG_MOUSE_BUTTON_LEFT)
#define SG_MOUSE_BUTTON_MMASK    SG_MOUSE_BUTTON(SG_MOUSE_BUTTON_MIDDLE)
#define SG_MOUSE_BUTTON_RMASK    SG_MOUSE_BUTTON(SG_MOUSE_BUTTON_RIGHT)
#define SG_MOUSE_BUTTON_X1MASK   SG_MOUSE_BUTTON(SG_MOUSE_BUTTON_X1)
#define SG_MOUSE_BUTTON_X2MASK   SG_MOUSE_BUTTON(SG_MOUSE_BUTTON_X2)