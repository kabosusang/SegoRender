#version 450
layout(binding = 0) uniform samplerCube cubemap;

layout (location = 0) in vec3 textureDir;

layout(location = 0) out vec4 outColor;
// Uniforms (push_constant)

void main(){
    outColor = texture(cubemap, textureDir);
}