#pragma once

#include "VulkanTest.h"

struct CubeVertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
};
struct BufferObject
{
    VkBuffer buffer;
    VkDeviceMemory  memory;
};
struct TextureObject
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
};
struct ShaderParameters
{
    glm::mat4 mtxWorld;
    glm::mat4 mtxView;
    glm::mat4 mtxProj;
};

class VKCubeRenderer : public VulkanTest
{
public:
	virtual void prepare() override;
	virtual void cleanup() override;
	virtual void makeCommand(VkCommandBuffer command) override;

private:
    void makeCubeGeometry();
    void prepareUniformBuffers();
    void prepareDescriptorSetLayout();
    void prepareDescriptorPool();
    void prepareDescriptorSet();

    BufferObject createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    TextureObject createTexture(const char* filepath);
    void setImageMemoryBarrier(VkCommandBuffer command, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    VkSampler createSampler();
    VkPipelineShaderStageCreateInfo loadShaderModule(const char* fileName, VkShaderStageFlagBits stage);

private:
    BufferObject m_vertexBuffer;
    BufferObject m_indexBuffer;
    uint32_t m_indexCount = 0;

    TextureObject m_texture;
    VkSampler m_sampler;

    std::vector<BufferObject> m_uniformBuffers;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool  m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSet;

    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
};