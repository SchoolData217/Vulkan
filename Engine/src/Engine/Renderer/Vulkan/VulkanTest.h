#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

#include "Engine/Renderer/Vulkan/VulkanContext.h"

class VulkanTest
{
public:
	void Initialize(GLFWwindow* window);
	void Render();
	void Terminate();

protected:
	uint32_t searchGraphicsQueueIndex();
	void selectSurfaceFormat(uint32_t format);
	void createSwapchain(GLFWwindow* window);
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

	uint32_t m_graphicsQueueIndex;
	VkSurfaceKHR m_surface;
	VkSurfaceFormatKHR m_surfaceFormat;
	VkSurfaceCapabilitiesKHR  m_surfaceCaps;
	VkPresentModeKHR m_presentMode;
	VkSwapchainKHR m_swapchain;
	VkExtent2D m_swapchainExtent;
	VkImage m_depthBuffer;
	VkDeviceMemory m_depthBufferMemory;
	VkImageView m_depthBufferView;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainViews;
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_framebuffers;
	std::vector<VkCommandBuffer> m_commands;
	std::vector<VkFence> m_fences;
	VkSemaphore m_renderCompletedSem, m_presentCompletedSem;

	uint32_t m_imageIndex;
};