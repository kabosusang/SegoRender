#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../host_device.h"

layout(binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec3 fragNormal;
layout (location = 1) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

// Uniforms (push_constant)
layout(push_constant) uniform Material{ 
   layout(offset = 64) Material_toshader material;
} mt;

void main(){
   
   if (mt.material.UseSampler != 0){
      outColor = texture(texSampler, fragUv) * mt.material.baseColorFactor;
   }else{
      outColor = mt.material.baseColorFactor;
   }

}