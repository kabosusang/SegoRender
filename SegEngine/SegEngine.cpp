#include "SegEngine.h"
#include "Core/Log/Log.h"

namespace Sego{

#define BIND_EVENT_FN(x) std::bind(&SegEngine::x, this, std::placeholders::_1)

SegEngine* SegEngine::Instance_ = nullptr;

void SegEngine::Init(){
    Instance_ = this;
    Log::Log_Init();
    window_ = std::unique_ptr<Window>(Window::Create());
    window_->SetEventCallback(BIND_EVENT_FN(OnEvent));
}

void SegEngine::destory(){

}

void SegEngine::Run(){
    
   while(m_Running){
        for (Layer* layer : layerStack_)
            layer->OnUpdate();
        window_->OnUpdate();
   }
}

void SegEngine::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
    
    //SG_CORE_TRACE("{0}", e);

    for (auto it = layerStack_.end(); it != layerStack_.begin();)
    {
        (*--it)->OnEvent(e);
        if (e.Handled)
            break;
    }

}

bool SegEngine::OnWindowClose(WindowCloseEvent &e)
{
    m_Running = false;
    return true;
}

void SegEngine::PushLayer(Layer* layer)
{
    layerStack_.PushLayer(layer);
    layer->OnAttach();
}

void SegEngine::PushOverlay(Layer* overlay)
{
    layerStack_.PushOverlay(overlay);
    overlay->OnAttach();
}




} // namespace Sego
