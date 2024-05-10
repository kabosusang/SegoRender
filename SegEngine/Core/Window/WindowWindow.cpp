#include "WindowWindow.h"
#include "Core/Log/Log.h"
#include "Core/Event/AppWinEvent.h"
#include "Core/Event/KeyEvent.h"
#include "Core/Event/MouseEvent.h"
#include "Core/Vulkan/VulkanContext.hpp"

#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl2.h>

namespace Sego{

static bool s_SDLInitialized = false;

Window* Window::Create(const WindowProps &props)
{
    return new WindowsWindow(props);
}

WindowsWindow::WindowsWindow(const WindowProps &props)
{
    Init(props);
}

WindowsWindow::~WindowsWindow()
{
    delete context_;
    SDL_DestroyWindow(window_);
    SDL_Quit();
}
void WindowsWindow::OnUpdate()
{
    PollEvent();
}

void WindowsWindow::Init(const WindowProps &props)
{
    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;

    SG_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    window_ = SDL_CreateWindow(props.Title.c_str(),
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          m_Data.Width, m_Data.Height,
                                          SDL_WINDOW_SHOWN|SDL_WINDOW_VULKAN|SDL_WINDOW_RESIZABLE
                                          |SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        SDL_Log("create window failed");
        exit(2);
    }
   
    context_ = new VulkanContext(window_);
    context_->Init(); //Vulkan Context Init
    
    SDL_SetWindowData(window_, "EventData", &m_Data);
}

// Fake event Callback
void WindowsWindow::PollEvent(){
SDL_Event event;
while (SDL_PollEvent(&event)) {
       switch(event.type){
            //WINDOW EVENT
            case SDL_WINDOWEVENT :{
                if(event.window.event == SDL_WINDOWEVENT_RESIZED){
                    WindowData& data = *(WindowData*)SDL_GetWindowData(window_,"EventData");
                    data.Width = event.window.data1;
                    data.Height = event.window.data2;
                    WindowResizeEvent event_win(event.window.data1,event.window.data2);
                    data.EventCallback(event_win);
                }else if(event.window.event == SDL_WINDOWEVENT_CLOSE){
                    WindowData& data = *(WindowData*)SDL_GetWindowData(window_,"EventData");
                    WindowCloseEvent event_win;
                    data.EventCallback(event_win);
                }else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED){
                    WindowData& data = *(WindowData*)SDL_GetWindowData(window_,"EventData");
                    WindowMinEvent event_win;
                    event_win.is_Min = true;
                    data.EventCallback(event_win);
                }else if (event.window.event == SDL_WINDOWEVENT_RESTORED ){
                    WindowData& data = *(WindowData*)SDL_GetWindowData(window_,"EventData");
                    WindowMinEvent event_win;
                    event_win.is_Min = false;
                    data.EventCallback(event_win);
                }
            }
            break;
            //KEYBOARD EVENT
            case SDL_KEYDOWN:{
                if(event.key.repeat == 0 ){
                    KeyPressedEvent event_key(event.key.keysym.scancode,0);
                    m_Data.EventCallback(event_key);
                }else if(event.key.repeat ){
                    KeyPressedEvent event_key(event.key.keysym.scancode,event.key.repeat);
                    m_Data.EventCallback(event_key);
                }
            }
            break;
            case SDL_KEYUP:{
                KeyReleasedEvent event_key(event.key.keysym.scancode);
                m_Data.EventCallback(event_key);
            }
            break;
            //MOUSE Button Event
            case SDL_MOUSEBUTTONDOWN:{
                MouseButtonPressedEvent event_mouse(event.button.button);
                m_Data.EventCallback(event_mouse);
            }
            break;
            case SDL_MOUSEBUTTONUP:{
                MouseButtonReleasedEvent event_mouse(event.button.button);
                m_Data.EventCallback(event_mouse);
            }
            break;
            case SDL_MOUSEWHEEL:{
                MouseScrolledEvent event_mouse((float)event.wheel.preciseX,(float)event.wheel.preciseY);
                m_Data.EventCallback(event_mouse);
            }
            break;
            case SDL_MOUSEMOTION:{
                MouseMovedEvent event_mouse((float)event.motion.x,(float)event.motion.y);
                m_Data.EventCallback(event_mouse);
            }
            break;
           
            default:
            break;
       }
       ImGui_ImplSDL2_ProcessEvent(&event);
}

}

void WindowsWindow::Shutdown(){
    
}

bool WindowsWindow::IsWindowResize(){
    context_->RebuildSwapChain();
    
    return true;
}


}