#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec2 inColor;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

layout(Push_constant) uniform PushConstant{
    mat4 model;
} pc;


void main(){
    gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPosition,1.0);
    outNormal = inNormal;
    outUV = inTexCoord;
}
