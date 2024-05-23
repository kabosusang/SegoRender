/* Copyright (c) 2018-2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_GOOGLE_include_directive : require

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;

layout (push_constant) uniform UBOParams {
	layout(offset = 64) float exposure;
	float gamma;
} uboParams;


layout (set = 0, binding = 0) uniform samplerCube samplerEnv;

#include "../Pbr/includes/tonemapping.glsl"
#include "../Pbr/includes/srgbtolinear.glsl"

void main() 
{
	vec3 color = SRGBtoLINEAR(tonemap(textureLod(samplerEnv, inUVW, 1.5))).rgb;	
	outColor = vec4(color * 1.0, 1.0);
}