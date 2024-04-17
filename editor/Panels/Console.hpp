#pragma once

namespace Sego{

    class Console{
    public:
        Console() = default;

        void OnImGuiRender();
        void ShowLogText(const std::string& log);
         

    };



}
