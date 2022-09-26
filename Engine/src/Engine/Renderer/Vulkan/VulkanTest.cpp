#include "epch.h"
#include "VulkanTest.h"

#include <sstream>

using namespace std;

static const array<VkClearValue, 2> clearValue =
{
{
	{0.2f, 0.2f, 0.2f, 1.0f}, // for Color
	{1.0f, 0.0f } // for Depth
}
};

void VulkanTest::Initialize(GLFWwindow* window)
{
	m_Context.Init();

	m_graphicsQueueIndex = searchGraphicsQueueIndex();

	// サーフェース生成
	glfwCreateWindowSurface(Engine::VulkanContext::GetInstance(), window, nullptr, &m_surface);
	// サーフェースのフォーマット情報選択
	selectSurfaceFormat(VK_FORMAT_B8G8R8A8_UNORM);
	// サーフェースの能力値情報取得
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context.m_PhysicalDevice->m_PhysicalDevice, m_surface, &m_surfaceCaps);
	VkBool32 isSupport;
	vkGetPhysicalDeviceSurfaceSupportKHR(m_Context.m_PhysicalDevice->m_PhysicalDevice, m_graphicsQueueIndex, m_surface, &isSupport);

	createSwapchain(window);

	createDepthBuffer();

	createImageViews();

	createRenderPass();

	createFramebuffer();

	prepareCommandBuffers();

	prepareSemaphores();

	prepare();
}

void VulkanTest::Render()
{
	uint32_t nextImageIndex = 0;
	vkAcquireNextImageKHR(m_Context.m_Device->m_LogicalDevice, m_swapchain, UINT64_MAX, m_presentCompletedSem, VK_NULL_HANDLE, &nextImageIndex);
	auto commandFence = m_fences[nextImageIndex];
	vkWaitForFences(m_Context.m_Device->m_LogicalDevice, 1, &commandFence, VK_TRUE, UINT64_MAX);

	VkRenderPassBeginInfo renderPassBI{};
	renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBI.renderPass = m_renderPass;
	renderPassBI.framebuffer = m_framebuffers[nextImageIndex];
	renderPassBI.renderArea.offset = VkOffset2D{ 0, 0 };
	renderPassBI.renderArea.extent = m_swapchainExtent;
	renderPassBI.pClearValues = clearValue.data();
	renderPassBI.clearValueCount = uint32_t(clearValue.size());

	// コマンドバッファ・レンダーパス開始
	VkCommandBufferBeginInfo commandBI{};
	commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	auto& command = m_commands[nextImageIndex];
	vkBeginCommandBuffer(command, &commandBI);
	vkCmdBeginRenderPass(command, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

	m_imageIndex = nextImageIndex;
	makeCommand(command);

	// コマンド・レンダーパス終了
	vkCmdEndRenderPass(command);
	vkEndCommandBuffer(command);

	// コマンドを実行（送信)
	VkSubmitInfo submitInfo{};
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command;
	submitInfo.pWaitDstStageMask = &waitStageMask;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_presentCompletedSem;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_renderCompletedSem;
	vkResetFences(m_Context.m_Device->m_LogicalDevice, 1, &commandFence);
	vkQueueSubmit(m_Context.m_Device->m_GraphicsQueue, 1, &submitInfo, commandFence);

	// Present 処理
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &nextImageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderCompletedSem;
	vkQueuePresentKHR(m_Context.m_Device->m_GraphicsQueue, &presentInfo);
}

void VulkanTest::Terminate()
{
	vkDeviceWaitIdle(m_Context.m_Device->m_LogicalDevice);

	cleanup();

	vkFreeCommandBuffers(m_Context.m_Device->m_LogicalDevice, m_Context.m_Device->m_CommandPool, uint32_t(m_commands.size()), m_commands.data());
	m_commands.clear();

	vkDestroyRenderPass(m_Context.m_Device->m_LogicalDevice, m_renderPass, nullptr);
	for (auto& v : m_framebuffers)
	{
		vkDestroyFramebuffer(m_Context.m_Device->m_LogicalDevice, v, nullptr);
	}
	m_framebuffers.clear();

	vkFreeMemory(m_Context.m_Device->m_LogicalDevice, m_depthBufferMemory, nullptr);
	vkDestroyImage(m_Context.m_Device->m_LogicalDevice, m_depthBuffer, nullptr);
	vkDestroyImageView(m_Context.m_Device->m_LogicalDevice, m_depthBufferView, nullptr);

	for (auto& v : m_swapchainViews)
	{
		vkDestroyImageView(m_Context.m_Device->m_LogicalDevice, v, nullptr);
	}
	m_swapchainImages.clear();
	vkDestroySwapchainKHR(m_Context.m_Device->m_LogicalDevice, m_swapchain, nullptr);

	for (auto& v : m_fences)
	{
		vkDestroyFence(m_Context.m_Device->m_LogicalDevice, v, nullptr);
	}
	m_fences.clear();
	vkDestroySemaphore(m_Context.m_Device->m_LogicalDevice, m_presentCompletedSem, nullptr);
	vkDestroySemaphore(m_Context.m_Device->m_LogicalDevice, m_renderCompletedSem, nullptr);

	vkDestroySurfaceKHR(Engine::VulkanContext::GetInstance(), m_surface, nullptr);
}

uint32_t VulkanTest::searchGraphicsQueueIndex()
{
	uint32_t propCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_Context.m_PhysicalDevice->m_PhysicalDevice, &propCount, nullptr);
	vector<VkQueueFamilyProperties> props(propCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_Context.m_PhysicalDevice->m_PhysicalDevice, &propCount, props.data());

	uint32_t graphicsQueue = ~0u;
	for (uint32_t i = 0; i < propCount; ++i)
	{
		if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueue = i; break;
		}
	}
	return graphicsQueue;
}

void VulkanTest::selectSurfaceFormat(uint32_t format)
{
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context.m_PhysicalDevice->m_PhysicalDevice, m_surface, &surfaceFormatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context.m_PhysicalDevice->m_PhysicalDevice, m_surface, &surfaceFormatCount, formats.data());

	// 検索して一致するフォーマットを見つける.
	for (const auto& f : formats)
	{
		if (f.format == format)
		{
			m_surfaceFormat = f;
		}
	}
}

void VulkanTest::createSwapchain(GLFWwindow* window)
{
	auto imageCount = (std::max)(2u, m_surfaceCaps.minImageCount);
	auto extent = m_surfaceCaps.currentExtent;
	if (extent.width == ~0u)
	{
		// 値が無効なのでウィンドウサイズを使用する.
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		extent.width = uint32_t(width);
		extent.height = uint32_t(height);
	}
	uint32_t queueFamilyIndices[] = { m_graphicsQueueIndex };
	VkSwapchainCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = m_surface;
	ci.minImageCount = imageCount;
	ci.imageFormat = m_surfaceFormat.format;
	ci.imageColorSpace = m_surfaceFormat.colorSpace;
	ci.imageExtent = extent;
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	ci.preTransform = m_surfaceCaps.currentTransform;
	ci.imageArrayLayers = 1;
	ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.queueFamilyIndexCount = 0;
	ci.presentMode = m_presentMode;
	ci.oldSwapchain = VK_NULL_HANDLE;
	ci.clipped = VK_TRUE;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	auto result = vkCreateSwapchainKHR(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_swapchain);
	VK_CHECK_RESULT(result);
	m_swapchainExtent = extent;
}

void VulkanTest::createDepthBuffer()
{
	VkImageCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.imageType = VK_IMAGE_TYPE_2D;
	ci.format = VK_FORMAT_D32_SFLOAT;
	ci.extent.width = m_swapchainExtent.width;
	ci.extent.height = m_swapchainExtent.height;
	ci.extent.depth = 1;
	ci.mipLevels = 1;
	ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	ci.samples = VK_SAMPLE_COUNT_1_BIT;
	ci.arrayLayers = 1;
	auto result = vkCreateImage(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_depthBuffer);
	VK_CHECK_RESULT(result);

	VkMemoryRequirements reqs;
	vkGetImageMemoryRequirements(m_Context.m_Device->m_LogicalDevice, m_depthBuffer, &reqs);
	VkMemoryAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = reqs.size;
	ai.memoryTypeIndex = getMemoryTypeIndex(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(m_Context.m_Device->m_LogicalDevice, &ai, nullptr, &m_depthBufferMemory);
	vkBindImageMemory(m_Context.m_Device->m_LogicalDevice, m_depthBuffer, m_depthBufferMemory, 0);
}

uint32_t VulkanTest::getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps)
{
	uint32_t result = ~0u;
	for (uint32_t i = 0; i < m_Context.m_PhysicalDevice->m_MemoryProperties.memoryTypeCount; ++i)
	{
		if (requestBits & 1)
		{
			const auto& types = m_Context.m_PhysicalDevice->m_MemoryProperties.memoryTypes[i];
			if ((types.propertyFlags & requestProps) == requestProps)
			{
				result = i;
				break;
			}
		}
		requestBits >>= 1;
	}
	return result;
}

void VulkanTest::createImageViews()
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(m_Context.m_Device->m_LogicalDevice, m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Context.m_Device->m_LogicalDevice, m_swapchain, &imageCount, m_swapchainImages.data());
	m_swapchainViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = m_surfaceFormat.format;
		ci.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A,
		};
		ci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		ci.image = m_swapchainImages[i];
		auto result = vkCreateImageView(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_swapchainViews[i]);
		VK_CHECK_RESULT(result);
	}

	// for depthbuffer
	{
		VkImageViewCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = VK_FORMAT_D32_SFLOAT;
		ci.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A,
		};
		ci.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
		ci.image = m_depthBuffer;
		auto result = vkCreateImageView(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_depthBufferView);
		VK_CHECK_RESULT(result);
	}
}

void VulkanTest::createRenderPass()
{
	VkRenderPassCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	array<VkAttachmentDescription, 2> attachments;
	auto& colorTarget = attachments[0];
	auto& depthTarget = attachments[1];

	colorTarget = VkAttachmentDescription{};
	colorTarget.format = m_surfaceFormat.format;
	colorTarget.samples = VK_SAMPLE_COUNT_1_BIT;
	colorTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorTarget.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	depthTarget = VkAttachmentDescription{};
	depthTarget.format = VK_FORMAT_D32_SFLOAT;
	depthTarget.samples = VK_SAMPLE_COUNT_1_BIT;
	depthTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthTarget.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference{}, depthReference{};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc{};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &colorReference;
	subpassDesc.pDepthStencilAttachment = &depthReference;

	ci.attachmentCount = uint32_t(attachments.size());
	ci.pAttachments = attachments.data();
	ci.subpassCount = 1;
	ci.pSubpasses = &subpassDesc;

	auto result = vkCreateRenderPass(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_renderPass);
	VK_CHECK_RESULT(result);
}

void VulkanTest::createFramebuffer()
{
	VkFramebufferCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = m_renderPass;
	ci.width = m_swapchainExtent.width;
	ci.height = m_swapchainExtent.height;
	ci.layers = 1;
	m_framebuffers.clear();
	for (auto& v : m_swapchainViews)
	{
		array<VkImageView, 2> attachments;
		ci.attachmentCount = uint32_t(attachments.size());
		ci.pAttachments = attachments.data();
		attachments[0] = v;
		attachments[1] = m_depthBufferView;

		VkFramebuffer framebuffer;
		auto result = vkCreateFramebuffer(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &framebuffer);
		VK_CHECK_RESULT(result);
		m_framebuffers.push_back(framebuffer);
	}
}

void VulkanTest::prepareCommandBuffers()
{
	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = m_Context.m_Device->m_CommandPool;
	ai.commandBufferCount = uint32_t(m_swapchainViews.size());
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	m_commands.resize(ai.commandBufferCount);
	auto result = vkAllocateCommandBuffers(m_Context.m_Device->m_LogicalDevice, &ai, m_commands.data());
	VK_CHECK_RESULT(result);

	// コマンドバッファのフェンスも同数用意する.
	m_fences.resize(ai.commandBufferCount);
	VkFenceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (auto& v : m_fences)
	{
		result = vkCreateFence(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &v);
		VK_CHECK_RESULT(result);
	}
}

void VulkanTest::prepareSemaphores()
{
	VkSemaphoreCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_renderCompletedSem);
	vkCreateSemaphore(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_presentCompletedSem);
}