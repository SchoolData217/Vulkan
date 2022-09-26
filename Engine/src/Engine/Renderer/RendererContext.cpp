#include "epch.h"
#include "RendererContext.h"

#include "Vulkan/VulkanContext.h"

namespace Engine {

	Ref<RendererContext> RendererContext::Create()
	{
		return CreateRef<VulkanContext>();
	}

}