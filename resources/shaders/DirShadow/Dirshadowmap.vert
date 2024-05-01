#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

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
