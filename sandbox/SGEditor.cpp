#include "Sego.hpp"
#include "imgui.h"
#include "EditorLayer.h"

namespace Sego{
    
class Editor : public SegEngine{
public:
    Editor(){
        PushLayer(new EditorLayer());
    }
    ~Editor(){
    }
};

SegEngine* CreateSegEngine(){
    return new Editor();
}

}

int main(int argc, char** argv){
    auto engine = Sego::CreateSegEngine();
    engine->Init();
    engine->Run();
    engine->destory();
    delete engine;
    return 0;
}