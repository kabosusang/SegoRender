#pragma once

namespace Sego{

    class Input{
    public:
        static bool ISKeyPressed(int keycode);
        static bool IsMouseButtonPressed(int button);
        static float GetMouseX();
        static float GetMouseY();
        static std::pair<float, float> GetMousePos();
        static std::pair<uint32_t, uint32_t> GetWindowSize();
        
    };



}