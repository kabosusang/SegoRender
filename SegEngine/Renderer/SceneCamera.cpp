#include "pch.h"
#include "SceneCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Sego{

    SceneCamera::SceneCamera(){
        RecalculateProjection();
    }

    void SceneCamera::SetOrthographic(float size, float nearClip, float farClip){
        m_OrthographicSize = size;
        m_OrthographicNear = nearClip;
        m_OrthographicFar = farClip;
    
        RecalculateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width,uint32_t height){
        m_AspectRation = (float)width / (float)height;

        RecalculateProjection();
    }

    void SceneCamera::RecalculateProjection(){
        float orthoLeft = -m_OrthographicSize * m_AspectRation * 0.5f;
        float orthoRight = m_OrthographicSize * m_AspectRation * 0.5f;
        float orthoBottom = -m_OrthographicSize * 0.5f;
        float orthoTop = m_OrthographicSize * 0.5f;

        m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
    }



}
