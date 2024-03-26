#pragma once
#include "render_pass.hpp"

namespace Sego{

    class UiPass : public RenderPass{
    public:
        virtual void Init() override;
        virtual void Render() override;
        virtual void destroy() override;

        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;
        void createDescriptorPool();
    private:
        std::vector<vk::Framebuffer> framebuffers_;
    };






}