/* Copyright (c) 2018-2023, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

struct ShaderMaterial {
	vec4 baseColorFactor;
	vec4 emissiveFactor;
	vec4 diffuseFactor;
	vec4 specularFactor;
	float workflow;
	int baseColorTextureSet;
	int physicalDescriptorTextureSet;
	int normalTextureSet;	
	int occlusionTextureSet;
	int emissiveTextureSet;
	float metallicFactor;	
	float roughnessFactor;	
	float alphaMask;	
	float alphaMaskCutoff;
	float emissiveStrength;
};

struct MaterialInfo{
	int MaterialIndex;
    vec4 baseColor;//模型颜色
    float metallic; //金属度
    float roughness;//粗糙度
    vec4 emissive;//自发光颜色
    float emissiveStrength;//自发光强度
};
