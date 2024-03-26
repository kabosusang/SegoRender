#pragma once
#include "Core/Window/Window.h"
#include "Core/Event/AppWinEvent.h"
#include "Core/Layer/LayerStack.h"
#include "Core/Base/Time.hpp"
#include "Imgui/ImGuiLayer.hpp"

namespace Sego{

class SegEngine{
public:
    SegEngine();
    virtual ~SegEngine() = default;

    void Init();
    void destory();
    void Run();
    inline void Close() { m_Running = false; }
    void OnEvent(Event& e);
    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);


    inline Window& GetWindow() { return *window_; }
    inline static SegEngine& Instance(){ return *Instance_;}
    
private:
    //Event
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);
    bool OnWindowMin(WindowMinEvent &e);
private:
    //WINDOW
    std::unique_ptr<Window> window_;
    std::unique_ptr<ImGuiLayer> imguiLayer_;
    bool m_Running = true;
    bool is_Min = false;
    LayerStack layerStack_;

    float LastFrameTime_ = 0.0f;
private:
    static SegEngine* Instance_;
};


}
