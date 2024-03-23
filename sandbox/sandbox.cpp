#include "Sego.hpp"
#include "imgui.h"

class ExampleLayer : public Sego::Layer{

public:
    ExampleLayer()
        : Layer("Example")
    {
    }

    void OnUpdate() override
    {
        if(Sego::Input::ISKeyPressed(KeyScand::SG_KEY_TAB))
            SG_TRACE("Tab Key is Pressed");
    
    }

    virtual void OnImGuiRender() override
    {
        ImGui::Begin("Test");
        ImGui::Text("Hello World");
        ImGui::End();
    }

    void OnEvent(Sego::Event& event) override
    {
        
        //SG_TRACE("{0}", event);
    }

};

class Sandbox : public Sego::SegEngine{
public:
    Sandbox(){
        PushLayer(new ExampleLayer());
        
    }
    ~Sandbox(){
    }
};

Sego::SegEngine* CreateSegEngine(){
    return new Sandbox();
}


int main(int argc, char** argv){
    auto engine = CreateSegEngine();
    engine->Init();
    engine->Run();
    engine->destory();
    delete engine;
    return 0;
}