#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../light.h"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;

struct VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 uv;
    vec4 FragPosLightSpace;
};

layout(location = 0) out VS_OUT vs_out;

layout(Push_constant) uniform PushConstant{
    mat4 mvp;
} pc;

layout(set = 0,binding = 0) uniform lightspace_vt{
    mat4 model;
    mat4 lightSpaceMatrix;
}local;

void main(){
    gl_Position = pc.mvp * vec4(inPosition, 1.0);

    //vs_out.FragPos = inPosition;
    //vs_out.Normal = inNormal;
    vs_out.FragPos = vec3(local.model * vec4(inPosition, 1.0));
    vs_out.Normal = transpose(inverse(mat3(local.model))) * inNormal;
    vs_out.uv = inTexCoord;
    vs_out.FragPosLightSpace = local.lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
}
