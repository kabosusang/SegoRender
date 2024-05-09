#version 450
layout(binding = 0) uniform samplerCube cubemap;

layout (location = 0) in vec3 textureDir;

layout (location = 0) out vec4 gAlbedoSpec;

void main(){
    gAlbedoSpec.rgb = texture(cubemap, textureDir).rgb;
    gAlbedoSpec.a = 1.0;

}