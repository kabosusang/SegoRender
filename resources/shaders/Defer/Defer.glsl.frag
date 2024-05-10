#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../device.h"
#include "../light.h"

struct VS_OUT{
    vec3 FragPosWS;
    vec3 Normal;
    vec2 uv;
};

#define MAX_POINT_LIGHTS 8 //TOOD: Dynamic Poit light

layout(location = 0) in VS_OUT fs_in;
layout(set = 0,binding = 1) uniform sampler2D texSampler;

// Uniforms (push_constant)
layout(push_constant) uniform Material{ 
   layout(offset = 64) Material_toshader material;
} mt;

// Output
layout (location = 0) out vec4 gAlbedoSpec;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gNormal;

void main(){
    vec3 color;
    if (mt.material.UseSampler != 0){
        color = texture(texSampler, fs_in.uv).rgb;
    }else{
        color = mt.material.baseColorFactor.rgb;
    }

    gPosition = vec4(fs_in.FragPosWS,1.0);
    gNormal = normalize(vec4(fs_in.Normal,1.0));
    gAlbedoSpec.rgb = color;
    gAlbedoSpec.a = 1.0;
}
