#pragma once
#include "imnodes.h"

namespace Sego{

    class MaterialNodes{
    public:
        MaterialNodes() = default;

        void Init();
        void OnImGuiRender();   

    private:
    ImNodesMiniMapLocation minimap_location_ = ImNodesMiniMapLocation_BottomRight;
    };



}
