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

struct ShaderMaterial {
	vec4 baseColorFactor;
	vec4 emissiveFactor;
	vec4 diffuseFactor;
	vec4 specularFactor;
	float workflow;
	int baseColorTextureSet;
	int physicalDescriptorTextureSet;
	int normalTextureSet;	
	int occlusionTextureSet;
	int emissiveTextureSet;
	float metallicFactor;	
	float roughnessFactor;	
	float alphaMask;	
	float alphaMaskCutoff;
	float emissiveStrength;
};

struct MaterialInfo{
	vec3 position;
	vec3 normal;
	vec4 base_color;
	vec4 emissive_color;
	float metallic;
	float roughness;
	float occlusion;
};



#endif