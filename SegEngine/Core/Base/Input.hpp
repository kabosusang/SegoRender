#pragma once

namespace Sego{

    class Input{
    public:
        inline static bool ISKeyPressed(int keycode){return instance_->IsKeyPressedImpl(keycode);}
        inline static bool ISMouseButtonPressed(int button){return instance_->IsMouseButtonPressedImpl(button);}
        inline static float GetMouseX(){return instance_->GetMouseXImpl();}
        inline static float GetMouseY(){return instance_->GetMouseYImpl();}
        inline static std::pair<float, float> GetMousePos(){return instance_->GetMousePosImpl();}
        inline static std::pair<uint32_t, uint32_t> GetWindowSize(){return instance_->GetWindowSizeImpl();}

    protected:
        virtual bool IsKeyPressedImpl(int keycode) = 0;
        virtual bool IsMouseButtonPressedImpl(int button) = 0;
        virtual float GetMouseXImpl() = 0;
        virtual float GetMouseYImpl() = 0;
        virtual std::pair<float, float> GetMousePosImpl() = 0;
        virtual std::pair<uint32_t, uint32_t> GetWindowSizeImpl() = 0;

    private:
        static Input* instance_;
    };



}