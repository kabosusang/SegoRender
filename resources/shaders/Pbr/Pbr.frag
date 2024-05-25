/* Copyright (c) 2018-2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

// PBR shader based on the Khronos WebGL PBR implementation
// See https://github.com/KhronosGroup/glTF-WebGL-PBR
// Supports both metallic roughness and specular glossiness inputs

#version 450
#extension GL_GOOGLE_include_directive : require
#include "../light.h"
#include "includes/shadermaterial.glsl"

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inColor0;


// Scene PushConstatns
layout (push_constant) uniform UBOParams {
	layout(offset = 64) float exposure;
	float gamma;
	float debugViewInputs;
	int MaterialIndex;
} uboParams;

// ibl textures
layout (set = 0, binding = 2) uniform samplerCube samplerIrradiance;
layout (set = 0, binding = 3) uniform samplerCube prefilteredMap;
layout (set = 0, binding = 4) uniform sampler2D samplerBRDFLUT;
//Shadow Texture
layout(set = 0, binding = 5) uniform sampler2DArray directional_light_shadow_texture_sampler;
layout(set = 0, binding = 6) uniform samplerCube point_light_shadow_texture_samplers[MAX_POINT_LIGHT_NUM];
layout(set = 0, binding = 7) uniform sampler2D spot_light_shadow_texture_samplers[MAX_SPOT_LIGHT_NUM];

// Material bindings
// Textures
layout (set = 0, binding = 8) uniform sampler2D colorMap;
layout (set = 0, binding = 9) uniform sampler2D physicalDescriptorMap;
layout (set = 0, binding = 10) uniform sampler2D normalMap;
layout (set = 0, binding = 11) uniform sampler2D aoMap;
layout (set = 0, binding = 12) uniform sampler2D emissiveMap;

// Properties
layout(std430, set = 0, binding = 13) buffer SSBO
{
   ShaderMaterial materials[ ];
};
//Light
layout(set = 0,binding = 14) uniform _LightingUBO{
    LightingUBO lighting_ubo;
};


layout (location = 0) out vec4 outColor;

// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms, outlined in the Readme.MD Appendix.
struct PBRInfo
{
	float NdotL;                  // cos angle between normal and light direction
	float NdotV;                  // cos angle between normal and view direction
	float NdotH;                  // cos angle between normal and half vector
	float VdotH;                  // cos angle between view direction and half vector
	float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
	float metalness;              // metallic value at the surface
	vec3 reflectance0;            // full reflectance color (normal incidence angle)
	vec3 reflectance90;           // reflectance color at grazing angle
	float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
	vec3 diffuseColor;            // color contribution from diffuse lighting
	vec3 specularColor;           // color contribution from specular lighting
};

const float M_PI = 3.141592653589793;
const float c_MinRoughness = 0.04;

const float PBR_WORKFLOW_METALLIC_ROUGHNESS = 0.0;
const float PBR_WORKFLOW_SPECULAR_GLOSINESS = 1.0f;

#include "includes/tonemapping.glsl"
#include "includes/srgbtolinear.glsl"


// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal(ShaderMaterial material)
{
	// Perturb normal, see http://www.thetenthplanet.de/archives/1180
	vec3 tangentNormal = texture(normalMap, material.normalTextureSet == 0 ? inUV0 : inUV1).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(inWorldPos);
	vec3 q2 = dFdy(inWorldPos);
	vec2 st1 = dFdx(inUV0);
	vec2 st2 = dFdy(inUV0);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

const mat4 k_shadow_bias_mat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);

float textureProj(vec4 shadow_coord, vec2 offset, uint cascade_index)
{
	if (shadow_coord.z > 0.0 && shadow_coord.z < 1.0) 
	{
		float depth = texture(directional_light_shadow_texture_sampler, 
			vec3(shadow_coord.xy + offset, cascade_index)).r;
		if (depth < shadow_coord.z - DIRECTIONAL_LIGHT_SHADOW_BIAS) 
		{
			return 0.0;
		}
	}
	return 1.0;
}

float filterPCF(vec4 shadow_coord, uint cascade_index)
{
	ivec2 dim = textureSize(directional_light_shadow_texture_sampler, 0).xy;
	float dx = PCF_DELTA_SCALE / float(dim.x);
	float dy = PCF_DELTA_SCALE / float(dim.y);

	int count = 0;
	float shadow = 0.0;
	for (int x = -PCF_SAMPLE_RANGE; x <= PCF_SAMPLE_RANGE; ++x) 
	{
		for (int y = -PCF_SAMPLE_RANGE; y <= PCF_SAMPLE_RANGE; ++y) 
		{
			shadow += textureProj(shadow_coord, vec2(dx * x, dy * y), cascade_index);
			count++;
		}
	}
	return shadow / count;
}

// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
vec3 getIBLContribution(PBRInfo pbrInputs, vec3 n, vec3 reflection)
{
	float lod = (pbrInputs.perceptualRoughness * lighting_ubo.sky_light.prefilter_mip_levels);
	// retrieve a scale and bias to F0. See [1], Figure 3
	vec3 brdf = (texture(samplerBRDFLUT, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;
	vec3 diffuseLight = SRGBtoLINEAR(tonemap(texture(samplerIrradiance, n))).rgb;

	vec3 specularLight = SRGBtoLINEAR(tonemap(textureLod(prefilteredMap, reflection, lod))).rgb;

	vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
	vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

	// For presentation, this allows us to disable IBL terms
	// For presentation, this allows us to disable IBL terms
	return (diffuse + specular) * lighting_ubo.sky_light.color;
}

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 diffuse(PBRInfo pbrInputs)
{
	return pbrInputs.diffuseColor / M_PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs)
{
	return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
	float NdotL = pbrInputs.NdotL;
	float NdotV = pbrInputs.NdotV;
	float r = pbrInputs.alphaRoughness;

	float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
	float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
	float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
	float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
	return roughnessSq / (M_PI * f * f);
}

// Gets metallic factor from specular glossiness workflow inputs 
float convertMetallic(vec3 diffuse, vec3 specular, float maxSpecular) {
	float perceivedDiffuse = sqrt(0.299 * diffuse.r * diffuse.r + 0.587 * diffuse.g * diffuse.g + 0.114 * diffuse.b * diffuse.b);
	float perceivedSpecular = sqrt(0.299 * specular.r * specular.r + 0.587 * specular.g * specular.g + 0.114 * specular.b * specular.b);
	if (perceivedSpecular < c_MinRoughness) {
		return 0.0;
	}
	float a = c_MinRoughness;
	float b = perceivedDiffuse * (1.0 - maxSpecular) / (1.0 - c_MinRoughness) + perceivedSpecular - 2.0 * c_MinRoughness;
	float c = c_MinRoughness - perceivedSpecular;
	float D = max(b * b - 4.0 * a * c, 0.0);
	return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}
vec3 getLightContribution(PBRInfo pbr_info, vec3 n, vec3 v, vec3 l, vec3 c)
{
	vec3 h = normalize(l + v);
	pbr_info.NdotL = clamp(dot(n, l), 0.001, 1.0);
	pbr_info.NdotH = clamp(dot(n, h), 0.0, 1.0);
	pbr_info.VdotH = clamp(dot(v, h), 0.0, 1.0);

	// calculate the shading terms for the microfacet specular shading model
	vec3 F = specularReflection(pbr_info);
	float G = geometricOcclusion(pbr_info);
	float D = microfacetDistribution(pbr_info);

	// calculation of analytical lighting contribution
	vec3 diffuse_contrib = (1.0 - F) * diffuse(pbr_info);
	vec3 specular_contrib = F * G * D / (4.0 * pbr_info.NdotL * pbr_info.NdotV);

	// obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
	return pbr_info.NdotL * c * (diffuse_contrib + specular_contrib);
}


void main()
{
	ShaderMaterial material = materials[uboParams.MaterialIndex];

	float perceptualRoughness;
	float metallic;
	vec3 diffuseColor;
	vec4 baseColor;

	vec3 f0 = vec3(0.04);

	if (material.alphaMask == 1.0f) {
		if (material.baseColorTextureSet > -1) {
			baseColor = SRGBtoLINEAR(texture(colorMap, material.baseColorTextureSet == 0 ? inUV0 : inUV1)) * material.baseColorFactor;
		} else {
			baseColor = material.baseColorFactor;
		}
		if (baseColor.a < material.alphaMaskCutoff) {
			discard;
		}
	}

	if (material.workflow == PBR_WORKFLOW_METALLIC_ROUGHNESS) {
		// Metallic and Roughness material properties are packed together
		// In glTF, these factors can be specified by fixed scalar values
		// or from a metallic-roughness map
		perceptualRoughness = material.roughnessFactor;
		metallic = material.metallicFactor;
		if (material.physicalDescriptorTextureSet > -1) {
			// Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
			// This layout intentionally reserves the 'r' channel for (optional) occlusion map data
			vec4 mrSample = texture(physicalDescriptorMap, material.physicalDescriptorTextureSet == 0 ? inUV0 : inUV1);
			perceptualRoughness = mrSample.g * perceptualRoughness;
			metallic = mrSample.b * metallic;
		} else {
			perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);
			metallic = clamp(metallic, 0.0, 1.0);
		}
		// Roughness is authored as perceptual roughness; as is convention,
		// convert to material roughness by squaring the perceptual roughness [2].

		// The albedo may be defined from a base texture or a flat color
		if (material.baseColorTextureSet > -1) {
			baseColor = SRGBtoLINEAR(texture(colorMap, material.baseColorTextureSet == 0 ? inUV0 : inUV1)) * material.baseColorFactor;
		} else {
			baseColor = material.baseColorFactor;
		}
	}

	if (material.workflow == PBR_WORKFLOW_SPECULAR_GLOSINESS) {
		// Values from specular glossiness workflow are converted to metallic roughness
		if (material.physicalDescriptorTextureSet > -1) {
			perceptualRoughness = 1.0 - texture(physicalDescriptorMap, material.physicalDescriptorTextureSet == 0 ? inUV0 : inUV1).a;
		} else {
			perceptualRoughness = 0.0;
		}

		const float epsilon = 1e-6;

		vec4 diffuse = SRGBtoLINEAR(texture(colorMap, inUV0));
		vec3 specular = SRGBtoLINEAR(texture(physicalDescriptorMap, inUV0)).rgb;

		float maxSpecular = max(max(specular.r, specular.g), specular.b);

		// Convert metallic value from specular glossiness inputs
		metallic = convertMetallic(diffuse.rgb, specular, maxSpecular);

		vec3 baseColorDiffusePart = diffuse.rgb * ((1.0 - maxSpecular) / (1 - c_MinRoughness) / max(1 - metallic, epsilon)) * material.diffuseFactor.rgb;
		vec3 baseColorSpecularPart = specular - (vec3(c_MinRoughness) * (1 - metallic) * (1 / max(metallic, epsilon))) * material.specularFactor.rgb;
		baseColor = vec4(mix(baseColorDiffusePart, baseColorSpecularPart, metallic * metallic), diffuse.a);

	}

	baseColor *= inColor0;

	diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;
	
	float alphaRoughness = perceptualRoughness * perceptualRoughness;

	vec3 specularColor = mix(f0, baseColor.rgb, metallic);

	// Compute reflectance.
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

	// For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
	// For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	vec3 n = (material.normalTextureSet > -1) ? getNormal(material) : normalize(inNormal);
	
	vec3 v = normalize(lighting_ubo.camera_pos - inWorldPos);    // Vector from surface point to camera
	vec3 reflection = normalize(reflect(-v, n));
	float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);

	PBRInfo pbrInputs;
	pbrInputs.NdotV = NdotV;
	pbrInputs.perceptualRoughness = perceptualRoughness;
	pbrInputs.metalness = metallic;
	pbrInputs.reflectance0 = specularEnvironmentR0;
	pbrInputs.reflectance90 = specularEnvironmentR90;
	pbrInputs.diffuseColor = diffuseColor;
	pbrInputs.specularColor = specularColor;

	// calculate light contribution
	vec3 light_color = vec3(0.0);
	// directional light
	if (bool(lighting_ubo.has_directional_light))
	{
		DirectionalLight directional_light = lighting_ubo.directional_light;

		float shadow = 1.0;
		if (bool(directional_light.cast_shadow))
		{
			uint cascade_index = 0;
			vec3 view_pos = (lighting_ubo.camera_view * vec4(inWorldPos, 1.0)).xyz;
			for(uint i = 0; i < SHADOW_CASCADE_NUM - 1; ++i)
			{
				if(view_pos.z < directional_light.cascade_splits[i]) 
				{	
					cascade_index = i + 1;
				}
			}
			vec4 shadow_coord = (k_shadow_bias_mat * directional_light.cascade_view_projs[cascade_index]) * vec4(inWorldPos, 1.0);
			shadow_coord = shadow_coord / shadow_coord.w;
			shadow = filterPCF(shadow_coord, cascade_index);
		}
		
		light_color += getLightContribution(pbrInputs, n, v, -directional_light.direction, directional_light.color) * shadow;
	}

	// point lights
	for (int i = 0; i < lighting_ubo.point_light_num; ++i)
	{
		PointLight point_light = lighting_ubo.point_lights[i];
		
		float distance = distance(point_light.position, inWorldPos);
		if (distance < point_light.radius)
		{
			float attenuation = 1.0 / (1.0 + point_light.linear_attenuation * distance + 
	    		    point_light.quadratic_attenuation * (distance * distance));
			vec3 c = point_light.color * attenuation;
			vec3 l = normalize(point_light.position - inWorldPos);

			float shadow = 1.0;
			if (bool(point_light.cast_shadow))
			{
				vec3 sample_vector = inWorldPos - point_light.position;
				float depth = texture(point_light_shadow_texture_samplers[i], sample_vector).x;
				if (length(sample_vector) > depth)
				{
					shadow = 0.0;
				}
			}

			light_color += getLightContribution(pbrInputs, n, v, l, c) * shadow;
		}
	}

	// spot lights
	for (int i = 0; i < lighting_ubo.spot_light_num; ++i)
	{
		SpotLight spot_light = lighting_ubo.spot_lights[i];
		PointLight point_light = spot_light._pl;

		float distance = distance(point_light.position, inWorldPos);
		if (distance < point_light.radius)
		{
			float pl_attenuation = 1.0 / (1.0 + point_light.linear_attenuation * distance + 
	    		    point_light.quadratic_attenuation * (distance * distance));
			float theta = dot(normalize(inWorldPos - point_light.position), spot_light.direction);
			float epsilon = point_light.padding0 - point_light.padding1;
			float sl_attenuation = clamp((theta - point_light.padding1) / epsilon, 0.0, 1.0); 

			vec3 c = point_light.color * pl_attenuation * sl_attenuation;
			vec3 l = normalize(point_light.position - inWorldPos);

			float shadow = 1.0;
			if (bool(point_light.cast_shadow))
			{
				vec4 shadow_coord = (k_shadow_bias_mat * spot_light.view_proj) * vec4(inWorldPos, 1.0);
				shadow_coord = shadow_coord / shadow_coord.w;

				if (shadow_coord.z > 0.0 && shadow_coord.z < 1.0) 
				{
					float depth = texture(spot_light_shadow_texture_samplers[i], shadow_coord.xy).r;
					if (depth < shadow_coord.z - SPOT_LIGHT_SHADOW_BIAS) 
					{
						shadow = 0.0;
					}
				}
			}

			light_color += getLightContribution(pbrInputs, n, v, l, c) * shadow;
		}
	}

	// calculate ibl contribution
	if (bool(lighting_ubo.has_sky_light))
	{
		light_color += getIBLContribution(pbrInputs, n, reflection);
	}

	vec3 color = light_color;
	
	const float u_OcclusionStrength = 1.0f;
	// Apply optional PBR terms for additional (optional) shading
	if (material.occlusionTextureSet > -1) {
		float ao = texture(aoMap, (material.occlusionTextureSet == 0 ? inUV0 : inUV1)).r;
		color = mix(color, color * ao, u_OcclusionStrength);
	}

	vec3 emissive = material.emissiveFactor.rgb * material.emissiveStrength;
	if (material.emissiveTextureSet > -1) {
		emissive *= SRGBtoLINEAR(texture(emissiveMap, material.emissiveTextureSet == 0 ? inUV0 : inUV1)).rgb;
	};
	color += emissive;
	
	outColor = vec4(color, baseColor.a);

	// Shader inputs debug visualization
	if (uboParams.debugViewInputs > 0.0) {
		int index = int(uboParams.debugViewInputs);
		switch (index) {
			case 1:
				outColor.rgba = material.baseColorTextureSet > -1 ? texture(colorMap, material.baseColorTextureSet == 0 ? inUV0 : inUV1) : vec4(1.0f);
				break;
			case 2:
				outColor.rgb = (material.normalTextureSet > -1) ? texture(normalMap, material.normalTextureSet == 0 ? inUV0 : inUV1).rgb : normalize(inNormal);
				break;
			case 3:
				outColor.rgb = (material.occlusionTextureSet > -1) ? texture(aoMap, material.occlusionTextureSet == 0 ? inUV0 : inUV1).rrr : vec3(0.0f);
				break;
			case 4:
				outColor.rgb = (material.emissiveTextureSet > -1) ? texture(emissiveMap, material.emissiveTextureSet == 0 ? inUV0 : inUV1).rgb : vec3(0.0f);
				break;
			case 5:
				outColor.rgb = texture(physicalDescriptorMap, inUV0).bbb;
				break;
			case 6:
				outColor.rgb = texture(physicalDescriptorMap, inUV0).ggg;
				break;
		}
		outColor = SRGBtoLINEAR(outColor);
	}

}