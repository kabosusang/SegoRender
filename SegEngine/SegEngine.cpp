#include "SegEngine.h"
#include "Core/Log/Log.h"
#include "Core/Base/Input.hpp"


namespace Sego{

#define BIND_EVENT_FN(x) std::bind(&SegEngine::x, this, std::placeholders::_1)

SegEngine* SegEngine::Instance_ = nullptr;

SegEngine::SegEngine(){
    Instance_ = this;
    Log::Log_Init();
    window_ = std::unique_ptr<Window>(Window::Create());
    window_->SetEventCallback(BIND_EVENT_FN(OnEvent));
    
    imguiLayer_ = std::make_unique<ImGuiLayer>();
    PushOverlay(imguiLayer_.get());
}


void SegEngine::Init(){
   
}

void SegEngine::destory(){
    Log::destroy();

}

void SegEngine::Run(){
   while(m_Running){
        
        float time = Timer::GetTime();
        Timestep timestep = time - LastFrameTime_;
        LastFrameTime_ = time;

        imguiLayer_->Begin();
        for (Layer* layer : layerStack_)
            layer->OnImGuiRender();
        imguiLayer_->End();

        for (Layer* layer : layerStack_)
            layer->OnUpdate(timestep);

        window_->OnUpdate();
        
         if(is_Min){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
   }
}

void SegEngine::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
    dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));
    dispatcher.Dispatch<WindowMinEvent>(BIND_EVENT_FN(OnWindowMin));

    //SG_CORE_TRACE("{0}", e);

    for (auto it = layerStack_.end(); it != layerStack_.begin();)
    {
        (*--it)->OnEvent(e);
        if (e.Handled)
            break;
    }
}

bool SegEngine::OnWindowResize(WindowResizeEvent &e){
    window_->IsWindowResize();
    
    return true;
}

bool SegEngine::OnWindowClose(WindowCloseEvent &e){
    m_Running = false;
    return true;
}

bool SegEngine::OnWindowMin(WindowMinEvent &e){
    is_Min = e.is_Min;
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
