#pragma once

#include <vulkan/vulkan.h>

#include "Engine/Renderer/RendererContext.h"
#include "VulkanDevice.h"

namespace Engine {

	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext() = default;
		virtual ~VulkanContext();

	public:
		virtual void Init() override;

		static VkInstance GetInstance() { return s_VulkanInstance; }

	private:
		inline static VkInstance s_VulkanInstance;

		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		Ref<VulkanDevice> m_Device;

		VkDebugUtilsMessengerEXT m_DebugUtilsMessenger = VK_NULL_HANDLE;

		friend class VulkanTest;
		friend class VKCubeRenderer;
		friend class RendererTest;
	};

}