#version 450
layout(set = 0 ,binding = 0) uniform sampler2D SkyboxTexture;

layout(location = 0) out vec4 outColor;
// Uniforms (push_constant)
layout (location = 0) in vec2 inUV;


void main(){
    vec4 alb = texture(SkyboxTexture, inUV);
    outColor = vec4(alb.rgb,1.0);
}