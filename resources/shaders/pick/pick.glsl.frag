#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outEntity;

void main() {
    outEntity = fragColor / 0.6f;
}