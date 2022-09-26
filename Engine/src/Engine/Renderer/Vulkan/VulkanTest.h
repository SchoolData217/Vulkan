#pragma once

struct GLFWwindow;

namespace Engine {

	class VulkanTest
	{
	public:
		void Initialize(GLFWwindow* window);
		void Render();
		void Terminate();

	private:
		void initializeInstance();
		void selectPhysicalDevice();
		uint32_t searchGraphicsQueueIndex();
		void createLogicalDevice();
		void prepareCommandPool();
		void selectSurfaceFormat(uint32_t format);
		void createSwapchain(GLFWwindow* window);
		void createDepthBuffer();
		void createImageViews();
		void createRenderPass();
		void createFramebuffer();
		void prepareCommandBuffers();
		void prepareSemaphores();
		void prepare() {}

		void enableDebugReport();
	};

}