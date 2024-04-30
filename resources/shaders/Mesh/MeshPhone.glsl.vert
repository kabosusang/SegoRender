#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../light.h"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;

struct VS_OUT{
    vec3 outPosition;
    vec3 outPositionWS;
    vec3 OutNormal;
    vec2 outTexCoord;
    vec4 shadowmap_space;
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

    vs_out.outPosition = inPosition;
    vs_out.outPositionWS = (local.model * vec4(inPosition,1.0)).rgb;
    vs_out.OutNormal = inNormal;
    vs_out.outTexCoord = inTexCoord;
    vs_out.shadowmap_space = local.lightSpaceMatrix * vec4(vs_out.outPositionWS, 1.0);
    
}
