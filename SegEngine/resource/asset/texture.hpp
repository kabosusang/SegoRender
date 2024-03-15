#pragma once

#include "buffer.hpp"
#include "Core/Vulkan/include/descriptor_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <string_view>
#include <string>

namespace Sego{

class TextureManager;

class Texture final {
public:
    friend class TextureManager;
    ~Texture();

    vk::Image image;
    vk::DeviceMemory memory;
    vk::ImageView view;
    DescriptorSetManager::SetInfo set;
    uint32_t width_;
    uint32_t height_;
    uint32_t mipLevels_;
    void fromBufferData(
	    void *             buffer,
	    vk::DeviceSize     bufferSize,
	    vk::Format         format,
	    uint32_t           texWidth,
	    uint32_t           texHeight,
	    vk::Filter         filter          = vk::Filter::eLinear,
	    vk::ImageUsageFlags  imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
	    vk::ImageLayout      imageLayout     = vk::ImageLayout::eShaderReadOnlyOptimal);
    
private:
    Texture(std::string_view filename);
    void createImage(uint32_t w, uint32_t h);
    void createImage(uint32_t w, uint32_t h,uint32_t mipLevels,
vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage);


    void createImageView();
    void allocMemory();
    uint32_t queryImageMemoryIndex();
    void transitionImageLayoutFromUndefine2Dst();
    void transitionImageLayoutFromDst2Optimal();
    void transformData2Image(Buffer&, uint32_t w, uint32_t h);
    void updateDescriptorSet();
};

class TextureManager final {
public:
    static TextureManager& Instance() {
        if (!instance_) {
            instance_.reset(new TextureManager);
        }
        return *instance_;
    }

    Texture* Load(const std::string& filename);
    void Destroy(Texture*);
    void Clear();

private:
    static std::unique_ptr<TextureManager> instance_;
    std::vector<std::unique_ptr<Texture>> datas_;
};

}
