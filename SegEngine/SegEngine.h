#pragma once
#include "Core/Window/Window.h"
#include "Core/Event/AppWinEvent.h"
#include "Core/Layer/LayerStack.h"


namespace Sego{

class SegEngine{
public:
    SegEngine() = default;
    virtual ~SegEngine() = default;

    void Init();
    void destory();
    void Run();
    void OnEvent(Event& e);

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

    inline Window& GetWindow() { return *window_; }
    inline static SegEngine& Instance(){ return *Instance_;}
    
private:
    bool OnWindowClose(WindowCloseEvent& e);
    std::unique_ptr<Window> window_;
    bool m_Running = true;
    LayerStack layerStack_;
private:
    static SegEngine* Instance_;
};


}
