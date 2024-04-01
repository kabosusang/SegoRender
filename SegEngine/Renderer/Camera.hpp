#pragma once

#include <glm/glm.hpp>

namespace Sego{
    class Camera{
    public:
        Camera(const glm::mat4& projectionMatrix)
            : m_ProjectionMatrix(projectionMatrix) {}
    
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
    private:
        glm::mat4 m_ProjectionMatrix;
    };

}