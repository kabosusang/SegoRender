#pragma once

#include "Input.hpp"

namespace Sego{

    class WindowsInput : public Input{
        protected:
            virtual bool IsKeyPressedImpl(int keycode) override;
            virtual bool IsMouseButtonPressedImpl(int button) override;
            virtual float GetMouseXImpl() override;
            virtual float GetMouseYImpl() override;
            virtual std::pair<float, float> GetMousePosImpl() override;
            virtual std::pair<uint32_t, uint32_t> GetWindowSizeImpl() override;
    };


}