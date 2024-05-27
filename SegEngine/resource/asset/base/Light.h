#ifndef LIGHT_H
#define LIGHT_H

#ifdef __cplusplus
#include <glm/glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;

#endif
#define SHADOW_CASCADE_NUM 4
#define MAX_POINT_LIGHT_NUM 4
#define MAX_SPOT_LIGHT_NUM 4
#define PCF_DELTA_SCALE 0.75
#define PCF_SAMPLE_RANGE 1
#define SHADOW_FACE_NUM 6

#define DIRECTIONAL_LIGHT_SHADOW_BIAS 0.002
#define SPOT_LIGHT_SHADOW_BIAS 0.001
struct SkyLight
{
    vec3 color;
	float prefilter_mip_levels;
};


struct DirectionalLight
{
	vec3 direction; int cast_shadow;
	vec3 color; float padding0;
	mat4 cascade_view_projs[SHADOW_CASCADE_NUM];
	vec4 cascade_splits;
};

struct PointLight
{
	vec3 position; 
    float padding0; // inner_cutoff for SpotLight, cast shadow for PointLight

	vec3 color; 
    float padding1; // outer_cutoff for SpotLight

    float radius;
	float linear_attenuation;
	float quadratic_attenuation;
    int cast_shadow;
};

struct SpotLight
{
    PointLight _pl;
    vec3 direction; float padding0;
    mat4 view_proj;
};

struct LightingUBO
{
    // camera
    vec3 camera_pos;
    float exposure;
    mat4 camera_view;
    mat4 inv_camera_view_proj;

    // lights
    SkyLight sky_light;
    DirectionalLight directional_light;
    PointLight point_lights[MAX_POINT_LIGHT_NUM];
    SpotLight spot_lights[MAX_SPOT_LIGHT_NUM];

    int has_sky_light;
    int has_directional_light;
    int point_light_num;
    int spot_light_num;
};

struct ShadowCascadeUBO
{
    mat4 cascade_view_projs[SHADOW_CASCADE_NUM];
};

struct ShadowCubeUBO
{
    mat4 face_view_projs[SHADOW_FACE_NUM];
};




/// PhoneShader LightSetting
struct DirLight{
    vec3 direction; //Direction of the light
    float Identity; //Identity of the light (Now:Buffer   TODO:USE)
    vec3 viewPos; //Camera position
    float ambient; //Ambient light Buffer
    mat4 lightSpaceMatrix; //LightSpaceMatrix
};

struct LightSetting{ 
    int UseLight;
    int lightCount;
};

struct LightObj{
    DirLight dirLight;
    LightSetting lightSetting;
    //PointLight pointLight;
};


struct LightSpace{
    mat4 model;
    mat4 lightSpaceMatrix;
};

struct shadowConstans{
    mat4 LightSpaceMatrix;
};





#endif
