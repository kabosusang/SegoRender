#pragma once
#include "pch.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct StaticVertex{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 color;
};

struct SpriteVertex{
    glm::vec3 pos;
    glm::vec4 color;
    glm::vec2 uv;
};

struct CircleVertex{
    glm::vec3 WorldPosition;
    glm::vec3 LocalPosition;
    glm::vec4 color;
    float Thickness;
    float Fade;

    //Editor-only
    int EntityID;
};


