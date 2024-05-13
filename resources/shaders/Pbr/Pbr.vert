#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 Uv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUv;

layout(Push_constant) uniform PushConstant{
    mat4 mvp;
} pc;


void main() {
    gl_Position = pc.mvp * vec4(inPosition,1.0);
    fragColor = inColor;
    fragUv = Uv;
}