#include "shader_manager.hpp"
#include "Core/Log/Log.h"
#include "context.hpp"

namespace Sego{

    vk::PipelineShaderStageCreateInfo ShaderManager::LoadShader(const std::string& filename, 
    vk::ShaderStageFlagBits stage){
        std::vector<char> code = ReadWholeFile(filename);
        if(code.empty()){
            SG_ERROR("Failed to load shader: {}", filename);
        }
        vk::ShaderModuleCreateInfo CreateInfo{};
        CreateInfo.codeSize = code.size();
        CreateInfo.pCode = (std::uint32_t*)code.data();
        
        vk::ShaderModule shaderModule = Context::Instance().device.createShaderModule(CreateInfo);
        shaderModules_[filename] = shaderModule;
        vk::PipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.setModule(shaderModule)
                       .setPName("main")
                       .setStage(stage);
        return shaderStageInfo;
    }

    void ShaderManager::Destroy(){
       

    }

}