#pragma once
#include "render_pass.hpp"

namespace Sego{
    class BRDFLutPass :public RenderPass{
    public:
        BRDFLutPass();
        virtual void Init() override;
        virtual void Render() override;
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

        virtual void destroy() override;
        virtual void createDescriptorSetLayout();
        virtual void createPipelineLayouts();
        
        VmaImageViewSampler GetImageViewSampler(){return lutIv_;}
    private:
     
    private:
        vk::Format m_format;
        uint32_t lutsize_;

        //pick Entity Pass
        VmaImageViewSampler lutIv_;
        

    };



    
}