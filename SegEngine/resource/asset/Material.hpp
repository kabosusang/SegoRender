#pragma once
#include "Texture2D.hpp"
#include <glm/glm.hpp>

namespace Sego{

 struct Material {
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		uint32_t baseColorTextureIndex;
	};


struct imageIndex{
	int32_t imageIndex;
};


/// PBRMaterial
class PbrMaterial{
    public:
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