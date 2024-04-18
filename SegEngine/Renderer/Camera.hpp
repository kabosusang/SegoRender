#pragma once
#include <glm/glm.hpp>
#include <resource/asset/CubeTexture.hpp>

namespace Sego{
    class Camera{
    public:
        Camera() = default;
        Camera(const glm::mat4& projectionMatrix)
            : m_Projection(projectionMatrix) {}
    
        virtual ~Camera() = default;

        const glm::mat4& GetProjection() const { return m_Projection; }
    protected:
        glm::mat4 m_Projection;
        std::shared_ptr<TextureCube> m_Skybox;
    };

}