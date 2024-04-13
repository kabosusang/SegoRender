#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 0) out int outEntity;

layout(push_constant) uniform PushConstant {
    layout(offset = 64) int entityID;
} pc;


void main() {
    outEntity = pc.entityID;
}