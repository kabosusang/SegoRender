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

struct PointLight{
    vec3 position; //Position of the light
    vec3 viewPos; //Camera position
    float constant; //Constant attenuation
    float linear; //Linear attenuation
    float quadratic; //Quadratic attenuation
};

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
    alignas(16) LightSetting lightSetting;
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
