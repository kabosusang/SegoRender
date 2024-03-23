#pragma once
#include "Core/Layer/Layer.h"
#include "Core/Event/KeyEvent.h"
#include "Core/Event/MouseEvent.h"
#include "Core/Event/AppWinEvent.h"

namespace Sego{

    class ImGuiLayer : public Layer{

    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnAttach() override;
        void OnDetach() override;
        void OnEvent(Event& event) override;
        virtual void OnImGuiRender() override;

        void Begin();
        void End();
    private:
   
    };


}