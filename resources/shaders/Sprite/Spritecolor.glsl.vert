#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
} ubo;

layout(Push_constant) uniform PushConstant{
    mat4 model;
} pc;


void main() {
    gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPosition,1.0);
    fragColor = inColor;
}