#include "Sego.hpp"

class ExampleLayer : public Sego::Layer{

public:
    ExampleLayer()
        : Layer("Example")
    {
    }

    void OnUpdate() override
    {
    
    }
    void OnEvent(Sego::Event& event) override
    {
        SG_TRACE("{0}", event);
    }

};

class Sandbox : public Sego::SegEngine{
public:
    Sandbox(){
        PushLayer(new ExampleLayer());
        PushOverlay(new Sego::ImGuiLayer());
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