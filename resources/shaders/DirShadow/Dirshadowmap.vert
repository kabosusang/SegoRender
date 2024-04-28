#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;

layout(Push_constant) uniform PushConstant{
    mat4 mvp;
} pc;

void main(){
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
}
