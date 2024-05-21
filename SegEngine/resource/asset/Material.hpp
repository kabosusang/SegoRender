#pragma once
#include "Texture2D.hpp"
#include <glm/glm.hpp>

namespace Sego{

struct Material {
	glm::vec4 baseColorFactor = glm::vec4(1.0f);

	/*
	glm::vec4 emissiveFactor = glm::vec4(0.0f);
	float metallicFactor = 1.0f;
	float roughnessFactor = 1.0f;
	float occlusionStrength = 1.0f;
	float alphaCutoff = 0.5f;*/

	int32_t has_baseColorTexture = 0;
	uint32_t baseColorTextureIndex; // index in textures array
};


struct imageIndex{
	int32_t imageIndex;
};

enum PBRWorkflows{ PBR_WORKFLOW_METALLIC_ROUGHNESS = 0, PBR_WORKFLOW_SPECULAR_GLOSINESS = 1 };
/// PBRMaterial
struct alignas(16) ShaderMaterial {
	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;
	glm::vec4 diffuseFactor;
	glm::vec4 specularFactor;
	float workflow;
	int colorTextureSet;
	int PhysicalDescriptorTextureSet;
	int normalTextureSet;
	int occlusionTextureSet;
	int emissiveTextureSet;
	float metallicFactor;
	float roughnessFactor;
	float alphaMask;
	float alphaMaskCutoff;
	float emissiveStrength;
};


}
