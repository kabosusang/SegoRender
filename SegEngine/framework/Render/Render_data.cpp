#include "Render_data.hpp"

//IBL
#include "framework/Render/pass/brdf_lut_pass.hpp"
#include "framework/Render/pass/filter_cube_pass.hpp"

namespace Sego{


    void SkyLightRenderData::CreateIBLTexture()
    {
        if (!std::filesystem::exists(LUTPATH)){
            std::shared_ptr<BRDFLutPass> brdf_pass = std::make_shared<BRDFLutPass>();
            brdf_pass->Init();
            brdf_pass->Render();
            brdf_pass->destroy();
        }
        std::shared_ptr<Texture2D> brdf_lut = std::make_shared<Texture2D>();
        std::string temp = LUTPATH;
        brdf_lut->loadFormFileBiranry(temp,vk::Format::eR16G16Sfloat,2048,2048,vk::SamplerAddressMode::eClampToEdge);

        lutBrdfIVs_ = brdf_lut->image_view_sampler_;
        std::shared_ptr<FilterCubePass> filter_pass = std::make_shared<FilterCubePass>(textureCube);
        filter_pass->Init();
        filter_pass->Render();
        filter_pass->destroy();

        irradianceIVs_.destroy();
		prefilteredIVs_.destroy();

        cube_mesh = filter_pass->getCubeMesh();
        irradianceIVs_ = filter_pass->getirradianceTexture();
        prefilteredIVs_ = filter_pass->getPrefilterTexture();
        m_prefilter_mip_levels = filter_pass->getPrefilterMipLevels();
    }

}