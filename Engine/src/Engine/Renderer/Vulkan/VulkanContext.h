#pragma once

#include <vulkan/vulkan.h>

#include "Engine/Renderer/RendererContext.h"

namespace Engine {

	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext() = default;
		virtual ~VulkanContext();

	public:
		virtual void Init() override;

		static VkInstance GetInstance() { return s_VulkanInstance; }

	public:
		inline static VkInstance s_VulkanInstance;
	};

}