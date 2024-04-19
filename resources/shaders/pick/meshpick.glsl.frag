#version 450
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUv;

layout(location = 0) out int outEntity;

layout(push_constant) uniform PushConstant {
    layout(offset = 64) int entityID;
} pc;

void main() {
    outEntity = pc.entityID;
}