#ifndef HOST_DEVICE
#define HOST_DEVICE

#ifdef __cplusplus
#include <glm/glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

struct TransformPCO
{
    mat4 m;
    mat4 nm;
    mat4 mvp;
};

struct CircleOutput{
	vec3 LocalPosition;
	vec4 color;
	float Thickness;
	float Fade;
};

struct SceneRenderSettings{
    float exposure; //曝光
    float gamma;//伽马
};


#endif