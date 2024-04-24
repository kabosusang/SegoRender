#version 450
#extension GL_GOOGLE_include_directive : enable
#include "../device.h"

struct VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 uv;
};

layout(location = 0) in VS_OUT fs_in;

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

// Uniforms (push_constant)
layout(push_constant) uniform Material{ 
   layout(offset = 64) LightAttribute Light_fs;
} L_fs;

void main(){
   vec3 color = texture(texSampler, fs_in.uv).rgb;

   //ambient
   vec3 ambient = 0.05 * color;
   //diffuse
   vec3 lightdir = normalize(L_fs.Light_fs.position - fs_in.FragPos);
   vec3 normal = normalize(fs_in.Normal);

   float diff = max(dot(normal, lightdir), 0.0);
   vec3 diffuse = diff * color;
   
   //specular
   vec3 viewDir = normalize(L_fs.Light_fs.viewPos - fs_in.FragPos);
   vec3 reflectDir = reflect(-lightdir, normal);
   float spec = 0.0;

   vec3 halfwayDir = normalize(lightdir + viewDir);  
   spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.3) * spec; // assuming bright white light color
    outColor = vec4(ambient + diffuse + specular, 1.0);
}