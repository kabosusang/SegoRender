#pragma once
#include "Renderer/Camera.hpp"


namespace Sego{

class Renderer{
public:
    void Init();

    void BeginScene(const Camera& camera, const glm::mat4& transform);
    void BeginScene();
    void EndScene();
    void Render();

};




}