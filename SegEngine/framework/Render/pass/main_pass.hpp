#pragma once
#include "render_pass.hpp"

namespace Sego{
    class MainPass : public RenderPass
    {
    public:
        virtual void Render() override;
      
        virtual void CreatePiepline() override;
        virtual void CreateFrameBuffer() override;
        virtual void CreateRenderPass() override;

    };




}