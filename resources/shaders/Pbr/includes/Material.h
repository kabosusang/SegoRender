#ifndef MATERIAL
#define MATERIAL
#include "../device.h"
#include "includes/srgbtolinear.glsl"


layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inColor0;

// Material bindings

// Properties
#include "includes/shadermaterial.glsl"
layout(std430, set = 0, binding = 2) buffer SSBO
{
   ShaderMaterial materials[ ];
};

//Material Index
layout (push_constant) uniform PushConstants {
	layout(offset = 64) int materialIndex;
} pushConstants;

// Textures
layout (set = 0, binding = 3) uniform sampler2D colorMap; 
layout (set = 0, binding = 4) uniform sampler2D physicalDescriptorMap;
layout (set = 0, binding = 5) uniform sampler2D normalMap;
layout (set = 0, binding = 6) uniform sampler2D aoMap;
layout (set = 0, binding = 7) uniform sampler2D emissiveMap;


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

MaterialInfo Compute_material()
{
    ShaderMaterial material = materials[pushConstants.materialIndex];
    MaterialInfo mat_info;
    float perceptualRoughness;
	float metallic;
	vec3 diffuseColor;
	vec4 baseColor;

    // position
    mat_info.position = inWorldPos;

    // normal
    mat_info.normal = (material.normalTextureSet > -1) ? getNormal(material) : normalize(inNormal);

   // base color
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
    //emissive Color
    vec3 emissive = material.emissiveFactor.rgb * material.emissiveStrength;
	if (material.emissiveTextureSet > -1) {
		emissive *= SRGBtoLINEAR(texture(emissiveMap, material.emissiveTextureSet == 0 ? inUV0 : inUV1)).rgb;
	};

    mat_info.emissive_color = vec4(emissive, 1.0f);

    // metallic_roughness_occlusion
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






}



#ifndef