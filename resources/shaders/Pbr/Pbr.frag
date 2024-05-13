#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform PushConstant {
    layout(offset = 64) int UseSampler;
} pc;

void main() {
    vec4 texColor = fragColor;
    if (pc.UseSampler != 0){
        texColor =  texture(texSampler, fragUv) * fragColor ;
        if (texColor.a < 0.1){
            discard;
        }
        outColor = texColor;
    }else{
        outColor = texColor;
    }

}