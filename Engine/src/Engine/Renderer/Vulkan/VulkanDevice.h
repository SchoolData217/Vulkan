#pragma once

#include <unordered_set>

#include "Vulkan.h"

class VulkanTest;

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

		friend class VulkanTest;
	};

}