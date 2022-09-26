#pragma once

#include <unordered_set>

#include "Vulkan.h"

class VulkanTest;
class VKCubeRenderer;
class RendererTest;

namespace Engine {

	class VulkanPhysicalDevice
	{
		struct QueueFamilyIndices
		{
			int32_t Graphics = -1;
			int32_t Compute = -1;
			int32_t Transfer = -1;
		};

	public:
		VulkanPhysicalDevice();
		~VulkanPhysicalDevice() = default;

	public:
		bool VulkanPhysicalDevice::IsExtensionSupported(const std::string& extensionName) const;

		VkPhysicalDevice GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
		VkFormat GetDepthFormat() const { return m_DepthFormat; }

	public:
		static Ref<VulkanPhysicalDevice> Select();

	private:
		QueueFamilyIndices GetQueueFamilyIndices(int queueFlags);
		VkFormat FindDepthFormat() const;

	private:
		VkPhysicalDeviceProperties m_Properties;
		VkPhysicalDevice m_PhysicalDevice = nullptr;
		VkPhysicalDeviceFeatures m_Features;
		VkPhysicalDeviceMemoryProperties m_MemoryProperties;

		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::unordered_set<std::string> m_SupportedExtensions;

		QueueFamilyIndices m_QueueFamilyIndices;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

		friend class VulkanDevice;
		friend class VulkanTest;
	};

	class VulkanDevice
	{
	public:
		VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);
		~VulkanDevice() = default;

		void Destroy();

		const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice GetVulkanDevice() const { return m_LogicalDevice; }

	private:
		VkDevice m_LogicalDevice = nullptr;
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;

		VkCommandPool m_CommandPool = nullptr, m_ComputeCommandPool = nullptr;

		VkQueue m_GraphicsQueue;
		VkQueue m_ComputeQueue;

		bool m_EnableDebugMarkers = false;

		friend class VulkanTest;
		friend class VKCubeRenderer;
		friend class RendererTest;
	};

}