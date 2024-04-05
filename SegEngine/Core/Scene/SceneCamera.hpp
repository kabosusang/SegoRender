#pragma once
#include "Renderer/Camera.hpp"

namespace Sego{
    class SceneCamera : public Camera{
    public:
        enum class ProjectionType {Perspective = 0, Orthographic = 1};
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;

        void SetViewportSize(uint32_t width,uint32_t height);

        void SetOrthographic(float size, float nearClip, float farClip);
        void SetPerspective(float verticalFOV, float nearClip, float farClip);
        
        //Perspective
        float GerPerspectiveVerticalFOV() const {return m_PerspectiveFOV;}
        void SetPerspectiveVerticalFOV(float verticalFov)  { m_PerspectiveFOV = verticalFov;RecalculateProjection();}
        float GerPerspectiveNearClip() const {return m_PerspectiveNear;}
        void SetPerspectiveNearClip(float nearClip)  { m_PerspectiveNear = nearClip;RecalculateProjection();}
        float GerPerspectiveFarClip() const {return m_PerspectiveFar;}
        void SetPerspectiveFarClip(float farClip)  { m_PerspectiveFar = farClip;RecalculateProjection();}

        //Orthographic
        float GetOrthographicSize() const { return m_OrthographicSize; }
        void  SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
        float GetOrthographicNearClip() const {return m_OrthographicNear;}
        void SetOrthographicNearClip(float nearClip) {m_OrthographicNear = nearClip; RecalculateProjection();}
        float GetOrthographicFarClip() const {return m_OrthographicFar;}
        void SetOrthographicFarClip(float farClip) {m_OrthographicFar = farClip; RecalculateProjection();}

        ProjectionType GetProjectionType() const {return m_ProjectionType;}
        void SetProjectionType(ProjectionType type) {m_ProjectionType = type; RecalculateProjection();}
    private:
        void RecalculateProjection();
    private:
        ProjectionType m_ProjectionType = ProjectionType::Perspective;

        float m_PerspectiveFOV = glm::radians(45.0f);
        float m_PerspectiveNear = 0.01f,m_PerspectiveFar = 1000.0f;

        float m_OrthographicSize = 10.0f;
        float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;


        float m_AspectRation = 0.0f;
    };




}