#include "pch.h"
#include "../Utils/PlatformUtils.h"
#include "Core/Vulkan/VulkanContext.hpp"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <SDL_syswm.h>
#endif

namespace Sego {
    std::string FileDialogs::OpenFile(const char* filter) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(VulkanContext::Instance().GetWindowHandle(), &wmInfo);
#ifdef _WIN32
     


        OPENFILENAME ofn;
        char szFile[260] = {0};
        ZeroMemory(&ofn,sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = wmInfo.info.win.window;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.nFilterIndex  = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST  | OFN_NOCHANGEDIR;
        if  (GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        return std::string();
#endif      
    }


    std::string FileDialogs::SaveFile(const char* filter) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(VulkanContext::Instance().GetWindowHandle(), &wmInfo);

#ifdef _WIN32
        OPENFILENAME ofn;
        char szFile[260] = {0};
        ZeroMemory(&ofn,sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = wmInfo.info.win.window;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.nFilterIndex  = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST  | OFN_NOCHANGEDIR;
        if  (GetSaveFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }

        return std::string();
#endif

    }




}