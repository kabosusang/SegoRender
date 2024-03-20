#pragma once
#include "Core/Layer/Layer.h"
#include <vulkan/vulkan.hpp>

namespace Sego{

    class ImGuiLayer : public Layer{

    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
        void OnEvent(Event& event) override;
    private:
         


    };


}