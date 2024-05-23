#include "Render_data.hpp"

//IBL
#include "framework/Render/pass/brdf_lut_pass.hpp"
#include "framework/Render/pass/filter_cube_pass.hpp"

namespace Sego{
    SceneRenderSettings SceneRenderData;
    
    void SkyLightRenderData::CreateIBLTexture(std::shared_ptr<TextureCube>& textureCube)
    {
        if (!std::filesystem::exists(LUTPATH)){
            std::shared_ptr<BRDFLutPass> brdf_pass = std::make_shared<BRDFLutPass>();
            brdf_pass->Init();
            brdf_pass->Render();
            brdf_pass->destroy();
        }
        //std::string temp = LUTPATH;
        //std::shared_ptr<Texture2D> brdf_lut = Texture2D::Create(temp,vk::Format::eR16G16Sfloat);

        std::shared_ptr<BRDFLutPass> brdf_pass = std::make_shared<BRDFLutPass>();
        brdf_pass->Init();
        brdf_pass->Render();
        brdf_pass->destroy();
        lutBrdfIVs_ = brdf_pass->GetImageViewSampler();
        
        std::shared_ptr<FilterCubePass> filter_pass = std::make_shared<FilterCubePass>(textureCube);
        filter_pass->Init();
        filter_pass->Render();
        filter_pass->destroy();

        this->irradianceIVs_.destroy();
		this->prefilteredIVs_.destroy();

        this->cube_mesh = filter_pass->getCubeMesh();
        this->irradianceIVs_ = filter_pass->getirradianceTexture();
        this->prefilteredIVs_ = filter_pass->getPrefilterTexture();
        this->m_prefilter_mip_levels = filter_pass->getPrefilterMipLevels();
    }

}