#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform PushConstant {
    layout(offset = 64) int UseSampler;
} pc;

void main() {

    if (pc.UseSampler != 0){
        outColor =  texture(texSampler, fragUv) * fragColor ;
    }else{
        outColor = fragColor;
    }

}