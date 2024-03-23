#pragma once


namespace Sego{

    class GraphicsContext{
        public:

            virtual void Init() = 0;
            virtual void SwapBuffers() = 0;
            virtual bool RebuildSwapChain() = 0;

    };





}