#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

#include "Engine/Renderer/Vulkan/VulkanContext.h"
#include "Engine/Renderer/Vulkan/VulkanSwapChain.h"

class VulkanTest
{
public:
	void Initialize(GLFWwindow* window);
	void Render();
	void Terminate();

protected:
	void createDepthBuffer();
	uint32_t getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps);
	void createImageViews();
	void createRenderPass();
	void createFramebuffer();
	void prepareCommandBuffers();
	void prepareSemaphores();

	virtual void prepare() {}
	virtual void cleanup() {}
	virtual void makeCommand(VkCommandBuffer command) {}

protected:
	Engine::VulkanContext m_Context;
	Engine::VulkanSwapChain m_SwapChain;

	VkImage m_depthBuffer;
	VkDeviceMemory m_depthBufferMemory;
	VkImageView m_depthBufferView;

	uint32_t m_imageIndex = 0;
};