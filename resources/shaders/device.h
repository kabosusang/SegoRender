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

struct Material_toshader {
	vec4 baseColorFactor;

	/*
	glm::vec4 emissiveFactor = glm::vec4(0.0f);
	float metallicFactor = 1.0f;
	float roughnessFactor = 1.0f;
	float occlusionStrength = 1.0f;
	float alphaCutoff = 0.5f;*/

	int UseSampler;
	int baseColorTextureIndex; // index in textures array
};

struct CircleOutput{
	vec3 LocalPosition;
	vec4 color;
	float Thickness;
	float Fade;
};




#endif