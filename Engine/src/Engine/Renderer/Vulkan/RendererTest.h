#pragma once

#include "VulkanTest.h"

namespace Engine {

	struct BufferObject
	{
		VkBuffer buffer;
		VkDeviceMemory  memory;
	};

	class RendererTest : public VulkanTest
	{
	public:
		RendererTest() = default;

	protected:
		virtual void prepare() override;
		virtual void cleanup() override;
		virtual void makeCommand(VkCommandBuffer command) override;

	private:
		BufferObject createBuffer(uint32_t size, VkBufferUsageFlags usage);
		VkPipelineShaderStageCreateInfo loadShaderModule(const char* fileName, VkShaderStageFlagBits stage);

	private:
		BufferObject m_vertexBuffer;
		BufferObject m_indexBuffer;
		uint32_t m_indexCount = 0;

		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;
	};

}