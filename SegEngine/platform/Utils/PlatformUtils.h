#pragma once

#include <string>
namespace Sego{

    class FileDialogs{
    public:
        //Opens a file dialog and returns the path to the selected file
        // if renturn null string is returned, the user has canceled the dialog
        static std::string OpenFile(const char* filter);    
        static std::string SaveFile(const char* filter);   


    };



}