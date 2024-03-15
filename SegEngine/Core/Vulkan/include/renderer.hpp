#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "command_manager.hpp"
#include "swapchain.hpp"
#include "math.hpp"
#include <limits>
#include "framework/Render/UI/imgui_init.hpp"
#include "resource/asset/mesh.hpp"

namespace Sego {

class Renderer {
public:
    Renderer(int maxFlightCount);
    ~Renderer();

    void SetProject(int right, int left, int bottom, int top, int far, int near);
    void DrawTexture(const Rect&, Texture& texture);
    void DrawModel(Node node);
    void SetDrawColor(const Color&);
    void StartRender();
    void EndRender();
    
    void SetFrameBufferSize(uint32_t w, uint32_t h ) { width = w; height = h; framebufferResized = true;}
    bool framebufferResized = false;
    uint32_t width,height;

private:
    //ImGui
    ImguiInit ui;

    int maxFlightCount_;
    int curFrame_;
    uint32_t imageIndex_;
    std::vector<vk::Fence> fences_;
    std::vector<vk::Semaphore> imageAvaliableSems_;
    std::vector<vk::Semaphore> renderFinishSems_;
    std::vector<vk::CommandBuffer> cmdBufs_;
    
    std::unique_ptr<Buffer> verticesBuffer_;
    std::unique_ptr<Buffer> indicesBuffer_;
    Mat4 projectMat_;
    Mat4 viewMat_;
    std::vector<std::unique_ptr<Buffer>> uniformBuffers_;
    std::vector<std::unique_ptr<Buffer>> colorBuffers_;
    std::vector<std::unique_ptr<Buffer>> deviceUniformBuffers_;
    std::vector<std::unique_ptr<Buffer>> deviceColorBuffers_;
    
    std::vector<DescriptorSetManager::SetInfo> descriptorSets_;
    vk::Sampler sampler;

    void createFences();
    void createSemaphores();
    void createCmdBuffers();
    void createBuffers();
    void createUniformBuffers(int flightCount);
    void bufferData();
    void bufferVertexData();
    void bufferIndicesData();
    void bufferMVPData();
    void initMats();
    void updateDescriptorSets();
    void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);
    std::uint32_t queryBufferMemTypeIndex(std::uint32_t, vk::MemoryPropertyFlags);
    void recreateCmdBuffersAndDesetPool();

};

}
