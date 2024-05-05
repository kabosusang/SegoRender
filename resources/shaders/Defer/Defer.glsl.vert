#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../light.h"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;

struct VS_OUT{
    vec3 outPositionWS;
    vec3 outNormal;
    vec2 outTexCoord;
    vec4 shadowmap_space;
};

layout(location = 0) out VS_OUT vs_out;

layout(Push_constant) uniform PushConstant{
    mat4 model;
} pc;

layout(set = 0,binding = 0) uniform lightspace_vt{
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
}local;

const mat4 BiasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0);

vec4 ComputeShadowCoord()
{
	return (BiasMat * local.lightSpaceMatrix * pc.model * vec4(inPosition, 1.0));
}


void main(){
    gl_Position = local.projection * local.view * pc.model * vec4(inPosition, 1.0);
    vs_out.outPositionWS = (pc.model * vec4(inPosition, 1.0)).xyz;
    vs_out.outNormal = (pc.model * vec4(normalize(inNormal), 1.0)).rgb; 
    vs_out.outTexCoord = inTexCoord;
    vs_out.shadowmap_space = ComputeShadowCoord();
}


