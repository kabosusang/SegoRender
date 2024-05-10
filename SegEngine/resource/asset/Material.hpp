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


/// PBRMaterial
class PbrMaterial{
    public:
		enum AlphaMode{ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND};
		std::shared_ptr<Texture2D> m_base_color_texure;
		std::shared_ptr<Texture2D> m_metallic_roughness_occlusion_texure;
		std::shared_ptr<Texture2D> m_normal_texure;
		std::shared_ptr<Texture2D> m_emissive_texure;

		glm::vec4 m_base_color_factor = glm::vec4(1.0f);
		glm::vec4 m_emissive_factor = glm::vec4(0.0f);
		float m_metallic_factor = 1.0f;
		float m_roughness_factor = 1.0f;
		bool m_contains_occlusion_channel = false;
};






}