#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

class VulkanTest
{
public:
	void Initialize(GLFWwindow* window);
	void Render();
	void Terminate();

protected:
	void initializeInstance();
	void selectPhysicalDevice();
	uint32_t searchGraphicsQueueIndex();
	void createLogicalDevice();
	void prepareCommandPool();
	void selectSurfaceFormat(uint32_t format);
	void createSwapchain(GLFWwindow* window);
	void createDepthBuffer();
	uint32_t getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps);
	void createImageViews();
	void createRenderPass();
	void createFramebuffer();
	void prepareCommandBuffers();
	void prepareSemaphores();

	void checkResult(VkResult result);

	virtual void prepare() {}
	virtual void cleanup() {}
	virtual void makeCommand(VkCommandBuffer command) {}

	void enableDebugReport();
	void disableDebugReport();

protected:
	VkInstance m_instance;
	VkPhysicalDevice m_physDev;
	VkPhysicalDeviceMemoryProperties m_physMemProps;
	uint32_t m_graphicsQueueIndex;
	VkDevice m_device;
	VkQueue m_deviceQueue;
	VkCommandPool m_commandPool;
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