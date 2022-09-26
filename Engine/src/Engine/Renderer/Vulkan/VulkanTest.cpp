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

	m_SwapChain.Init(m_Context.GetInstance(), m_Context.m_Device);
	m_SwapChain.InitSurface(window);
	uint32_t width = 1600, height = 900;
	m_SwapChain.Create(&width, &height, true);

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
	vkAcquireNextImageKHR(m_Context.m_Device->m_LogicalDevice, m_SwapChain.m_SwapChain, UINT64_MAX, m_SwapChain.m_Semaphores.PresentComplete, VK_NULL_HANDLE, &nextImageIndex);
	auto commandFence = m_SwapChain.m_WaitFences[nextImageIndex];
	vkWaitForFences(m_Context.m_Device->m_LogicalDevice, 1, &commandFence, VK_TRUE, UINT64_MAX);

	VkRenderPassBeginInfo renderPassBI{};
	renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBI.renderPass = m_SwapChain.m_RenderPass;
	renderPassBI.framebuffer = m_SwapChain.m_Framebuffers[nextImageIndex];
	renderPassBI.renderArea.offset = { 0, 0 };
	renderPassBI.renderArea.extent = { m_SwapChain.m_Width, m_SwapChain.m_Height };
	renderPassBI.pClearValues = clearValue.data();
	renderPassBI.clearValueCount = uint32_t(clearValue.size());

	// コマンドバッファ・レンダーパス開始
	VkCommandBufferBeginInfo commandBI{};
	commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	auto& command = m_SwapChain.m_CommandBuffers[nextImageIndex];
	vkBeginCommandBuffer(command.CommandBuffer, &commandBI);
	vkCmdBeginRenderPass(command.CommandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

	m_imageIndex = nextImageIndex;
	makeCommand(command.CommandBuffer);

	// コマンド・レンダーパス終了
	vkCmdEndRenderPass(command.CommandBuffer);
	vkEndCommandBuffer(command.CommandBuffer);

	// コマンドを実行（送信)
	VkSubmitInfo submitInfo{};
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command.CommandBuffer;
	submitInfo.pWaitDstStageMask = &waitStageMask;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_SwapChain.m_Semaphores.PresentComplete;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_SwapChain.m_Semaphores.RenderComplete;
	vkResetFences(m_Context.m_Device->m_LogicalDevice, 1, &commandFence);
	vkQueueSubmit(m_Context.m_Device->m_GraphicsQueue, 1, &submitInfo, commandFence);

	// Present 処理
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_SwapChain.m_SwapChain;
	presentInfo.pImageIndices = &nextImageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_SwapChain.m_Semaphores.RenderComplete;
	vkQueuePresentKHR(m_Context.m_Device->m_GraphicsQueue, &presentInfo);
}

void VulkanTest::Terminate()
{
	vkDeviceWaitIdle(m_Context.m_Device->m_LogicalDevice);

	cleanup();

	vkFreeMemory(m_Context.m_Device->m_LogicalDevice, m_depthBufferMemory, nullptr);
	vkDestroyImage(m_Context.m_Device->m_LogicalDevice, m_depthBuffer, nullptr);
	vkDestroyImageView(m_Context.m_Device->m_LogicalDevice, m_depthBufferView, nullptr);

	m_SwapChain.Destroy();
}

void VulkanTest::createDepthBuffer()
{
	VkImageCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.imageType = VK_IMAGE_TYPE_2D;
	ci.format = VK_FORMAT_D32_SFLOAT;
	ci.extent.width = m_SwapChain.m_Width;
	ci.extent.height = m_SwapChain.m_Height;
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
	colorTarget.format = m_SwapChain.m_ColorFormat;
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

	auto result = vkCreateRenderPass(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_SwapChain.m_RenderPass);
	VK_CHECK_RESULT(result);
}

void VulkanTest::createFramebuffer()
{
	VkFramebufferCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = m_SwapChain.m_RenderPass;
	ci.width = m_SwapChain.m_Width;
	ci.height = m_SwapChain.m_Height;
	ci.layers = 1;
	m_SwapChain.m_Framebuffers.clear();
	for (auto& v : m_SwapChain.m_Images)
	{
		array<VkImageView, 2> attachments;
		ci.attachmentCount = uint32_t(attachments.size());
		ci.pAttachments = attachments.data();
		attachments[0] = v.ImageView;
		attachments[1] = m_depthBufferView;

		VkFramebuffer framebuffer;
		auto result = vkCreateFramebuffer(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &framebuffer);
		VK_CHECK_RESULT(result);
		m_SwapChain.m_Framebuffers.push_back(framebuffer);
	}
}

void VulkanTest::prepareCommandBuffers()
{
	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = m_Context.m_Device->m_CommandPool;
	ai.commandBufferCount = uint32_t(m_SwapChain.m_Images.size());
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	m_SwapChain.m_CommandBuffers.resize(ai.commandBufferCount);
	auto result = vkAllocateCommandBuffers(m_Context.m_Device->m_LogicalDevice, &ai, &m_SwapChain.m_CommandBuffers[m_imageIndex].CommandBuffer);
	VK_CHECK_RESULT(result);

	// コマンドバッファのフェンスも同数用意する.
	m_SwapChain.m_WaitFences.resize(ai.commandBufferCount);
	VkFenceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (auto& v : m_SwapChain.m_WaitFences)
	{
		result = vkCreateFence(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &v);
		VK_CHECK_RESULT(result);
	}
}

void VulkanTest::prepareSemaphores()
{
	VkSemaphoreCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_SwapChain.m_Semaphores.RenderComplete);
	vkCreateSemaphore(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &m_SwapChain.m_Semaphores.PresentComplete);
}