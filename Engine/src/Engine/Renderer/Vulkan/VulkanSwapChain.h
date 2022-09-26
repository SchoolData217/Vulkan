#pragma once

#include "VulkanDevice.h"

struct GLFWwindow;

class VulkanTest;
class RendererTest;
class VKCubeRenderer;

namespace Engine {

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain() = default;
		~VulkanSwapChain() = default;

	public:
		void Init(VkInstance instance, const Ref<VulkanDevice>& device);
		void InitSurface(GLFWwindow* windowHandle);
		void Create(uint32_t* width, uint32_t* height, bool vsync);
		void Destroy();

	private:
		void FindImageFormatAndColorSpace();

	private:
		VkInstance m_Instance = nullptr;
		Ref<VulkanDevice> m_Device;

		VkSwapchainKHR m_SwapChain = nullptr;

		uint32_t m_ImageCount = 0;
		std::vector<VkImage> m_VulkanImages;

		bool m_VSync = false;
		uint32_t m_Width = 0, m_Height = 0;

		VkSurfaceKHR m_Surface;
		uint32_t m_QueueNodeIndex = UINT32_MAX;

		VkFormat m_ColorFormat;
		VkColorSpaceKHR m_ColorSpace;

		struct SwapchainImage
		{
			VkImage Image = nullptr;
			VkImageView ImageView = nullptr;
		};
		std::vector<SwapchainImage> m_Images;

		struct SwapchainCommandBuffer
		{
			VkCommandPool CommandPool = nullptr;
			VkCommandBuffer CommandBuffer = nullptr;
		};
		std::vector<SwapchainCommandBuffer> m_CommandBuffers;

		struct
		{
			// Swap chain
			VkSemaphore PresentComplete = nullptr;
			// Command buffer
			VkSemaphore RenderComplete = nullptr;
		} m_Semaphores;
		VkSubmitInfo m_SubmitInfo;

		std::vector<VkFence> m_WaitFences;

		VkRenderPass m_RenderPass = nullptr;
		//uint32_t m_CurrentBufferIndex = 0;
		//uint32_t m_CurrentImageIndex = 0;

		std::vector<VkFramebuffer> m_Framebuffers;

		friend class VulkanTest;
		friend class RendererTest;
		friend class VKCubeRenderer;
	};

}