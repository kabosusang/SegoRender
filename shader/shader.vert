#version 450

layout(location = 0) in vec2 Position;

layout(set = 0 ,binding = 1) uniform UniformBuffer{
    mat4 project;
    mat4 view;
    mat4 model;
}ubo;

void main() {
    gl_Position = ubo.project * ubo.view * ubo.model * vec4(Position, 0.0, 1.0);
}

