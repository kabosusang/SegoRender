#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;

struct VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 uv;
};

layout(location = 0) out VS_OUT vs_out;

layout(Push_constant) uniform PushConstant{
    mat4 mvp;
} pc;

void main(){
    vs_out.FragPos = inPosition;
    vs_out.Normal = inNormal;
    vs_out.uv = inTexCoord;

    gl_Position = pc.mvp * vec4(inPosition, 1.0);
  
}
