#include "epch.h"
#include "VulkanTest.h"

#include <sstream>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

using namespace std;

namespace {
#pragma region debug report
	static VkBool32 VKAPI_CALL DebugReportCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objactTypes,
		uint64_t object,
		size_t	location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		VkBool32 ret = VK_FALSE;
		if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT ||
			flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		{
			ret = VK_TRUE;
		}
		std::stringstream ss;
		if (pLayerPrefix)
		{
			ss << "[" << pLayerPrefix << "] ";
		}
		ss << pMessage << std::endl;

		OutputDebugStringA(ss.str().c_str());

		return ret;
	}

	PFN_vkCreateDebugReportCallbackEXT	m_vkCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT	m_vkDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallbackEXT;
	VkDebugReportCallbackEXT  m_debugReport;


	void checkResult(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			DebugBreak();
		}
	}
#pragma endregion

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

	void MAKE_COMMAND(VkCommandBuffer command) { }


	uint32_t getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps)
	{
		uint32_t result = ~0u;
		for (uint32_t i = 0; i < m_physMemProps.memoryTypeCount; ++i)
		{
			if (requestBits & 1)
			{
				const auto& types = m_physMemProps.memoryTypes[i];
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

}

namespace Engine {

#define GetInstanceProcAddr(FuncName) \
  m_##FuncName = reinterpret_cast<PFN_##FuncName>(vkGetInstanceProcAddr(m_instance, #FuncName))

	void VulkanTest::Initialize(GLFWwindow* window)
	{
		initializeInstance();

		selectPhysicalDevice();

		m_graphicsQueueIndex = searchGraphicsQueueIndex();

#ifdef DEBUG
		enableDebugReport();
#endif

		createLogicalDevice();

		prepareCommandPool();

		// サーフェース生成
		glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
		// サーフェースのフォーマット情報選択
		selectSurfaceFormat(VK_FORMAT_B8G8R8A8_UNORM);
		// サーフェースの能力値情報取得
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physDev, m_surface, &m_surfaceCaps);
		VkBool32 isSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physDev, m_graphicsQueueIndex, m_surface, &isSupport);

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
		vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_presentCompletedSem, VK_NULL_HANDLE, &nextImageIndex);
		auto commandFence = m_fences[nextImageIndex];
		vkWaitForFences(m_device, 1, &commandFence, VK_TRUE, UINT64_MAX);

		// クリア値
		array<VkClearValue, 2> clearValue = {
		  { {0.0f, 1.0f, 0.0, 0.0f}, // for Color
			{1.0f, 0 } // for Depth
		  }
		};

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
		MAKE_COMMAND(command);

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
		vkResetFences(m_device, 1, &commandFence);
		vkQueueSubmit(m_deviceQueue, 1, &submitInfo, commandFence);

		// Present 処理
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &nextImageIndex;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_renderCompletedSem;
		vkQueuePresentKHR(m_deviceQueue, &presentInfo);
	}

	void VulkanTest::Terminate()
	{
	}

	void VulkanTest::initializeInstance()
	{
		std::vector<const char*> extensions;
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan";
		appInfo.pEngineName = "Vulkan";
		appInfo.apiVersion = VK_API_VERSION_1_1;
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);


		// 拡張情報の取得.
		std::vector<VkExtensionProperties> props;
		{
			uint32_t count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
			props.resize(count);
			vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data());

			for (const auto& v : props)
			{
				extensions.push_back(v.extensionName);
			}
		}


		VkInstanceCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		ci.enabledExtensionCount = uint32_t(extensions.size());
		ci.ppEnabledExtensionNames = extensions.data();
		ci.pApplicationInfo = &appInfo;
#ifdef _DEBUG 
		// デバッグビルド時には検証レイヤーを有効化
		const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
		if (VK_HEADER_VERSION_COMPLETE < VK_MAKE_VERSION(1, 1, 106)) {
			// "VK_LAYER_LUNARG_standard_validation" は廃止になっているが昔の Vulkan SDK では動くので対処しておく.
			layers[0] = "VK_LAYER_LUNARG_standard_validation";
		}
		ci.enabledLayerCount = 1;
		ci.ppEnabledLayerNames = layers;
#endif


		// インスタンス生成
		auto result = vkCreateInstance(&ci, nullptr, &m_instance);
		checkResult(result);
	}

	void VulkanTest::selectPhysicalDevice()
	{
		uint32_t devCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &devCount, nullptr);
		std::vector<VkPhysicalDevice> physDevs(devCount);
		vkEnumeratePhysicalDevices(m_instance, &devCount, physDevs.data());

		// 最初のデバイスを使用する
		m_physDev = physDevs[0];

		// メモリプロパティを取得しておく
		vkGetPhysicalDeviceMemoryProperties(m_physDev, &m_physMemProps);
	}

	uint32_t VulkanTest::searchGraphicsQueueIndex()
	{
		uint32_t propCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physDev, &propCount, nullptr);
		vector<VkQueueFamilyProperties> props(propCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physDev, &propCount, props.data());

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

	void VulkanTest::createLogicalDevice()
	{
		const float defaultQueuePriority(1.0f);
		VkDeviceQueueCreateInfo devQueueCI{};
		devQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		devQueueCI.queueFamilyIndex = m_graphicsQueueIndex;
		devQueueCI.queueCount = 1;
		devQueueCI.pQueuePriorities = &defaultQueuePriority;


		vector<VkExtensionProperties> devExtProps;
		{
			// 拡張情報の取得.
			uint32_t count = 0;
			vkEnumerateDeviceExtensionProperties(m_physDev, nullptr, &count, nullptr);
			devExtProps.resize(count);
			vkEnumerateDeviceExtensionProperties(m_physDev, nullptr, &count, devExtProps.data());
		}

		vector<const char*> extensions;
		for (const auto& v : devExtProps)
		{
			extensions.push_back(v.extensionName);
		}
		VkDeviceCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		ci.pQueueCreateInfos = &devQueueCI;
		ci.queueCreateInfoCount = 1;
		ci.ppEnabledExtensionNames = extensions.data();
		ci.enabledExtensionCount = uint32_t(extensions.size());

		auto result = vkCreateDevice(m_physDev, &ci, nullptr, &m_device);
		checkResult(result);

		// デバイスキューの取得
		vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_deviceQueue);
	}

	void VulkanTest::prepareCommandPool()
	{
		VkCommandPoolCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		ci.queueFamilyIndex = m_graphicsQueueIndex;
		ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		auto result = vkCreateCommandPool(m_device, &ci, nullptr, &m_commandPool);
		checkResult(result);
	}

	void VulkanTest::selectSurfaceFormat(uint32_t format)
	{
		uint32_t surfaceFormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physDev, m_surface, &surfaceFormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(surfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physDev, m_surface, &surfaceFormatCount, formats.data());

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

		auto result = vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapchain);
		checkResult(result);
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
		auto result = vkCreateImage(m_device, &ci, nullptr, &m_depthBuffer);
		checkResult(result);

		VkMemoryRequirements reqs;
		vkGetImageMemoryRequirements(m_device, m_depthBuffer, &reqs);
		VkMemoryAllocateInfo ai{};
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = reqs.size;
		ai.memoryTypeIndex = getMemoryTypeIndex(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkAllocateMemory(m_device, &ai, nullptr, &m_depthBufferMemory);
		vkBindImageMemory(m_device, m_depthBuffer, m_depthBufferMemory, 0);
	}

	void VulkanTest::createImageViews()
	{
		uint32_t imageCount;
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
		m_swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
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
			auto result = vkCreateImageView(m_device, &ci, nullptr, &m_swapchainViews[i]);
			checkResult(result);
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
			auto result = vkCreateImageView(m_device, &ci, nullptr, &m_depthBufferView);
			checkResult(result);
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

		auto result = vkCreateRenderPass(m_device, &ci, nullptr, &m_renderPass);
		checkResult(result);
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
			auto result = vkCreateFramebuffer(m_device, &ci, nullptr, &framebuffer);
			checkResult(result);
			m_framebuffers.push_back(framebuffer);
		}
	}

	void VulkanTest::prepareCommandBuffers()
	{
		VkCommandBufferAllocateInfo ai{};
		ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		ai.commandPool = m_commandPool;
		ai.commandBufferCount = uint32_t(m_swapchainViews.size());
		ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		m_commands.resize(ai.commandBufferCount);
		auto result = vkAllocateCommandBuffers(m_device, &ai, m_commands.data());
		checkResult(result);

		// コマンドバッファのフェンスも同数用意する.
		m_fences.resize(ai.commandBufferCount);
		VkFenceCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (auto& v : m_fences)
		{
			result = vkCreateFence(m_device, &ci, nullptr, &v);
			checkResult(result);
		}
	}

	void VulkanTest::prepareSemaphores()
	{
		VkSemaphoreCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(m_device, &ci, nullptr, &m_renderCompletedSem);
		vkCreateSemaphore(m_device, &ci, nullptr, &m_presentCompletedSem);
	}

	void VulkanTest::enableDebugReport()
	{
		GetInstanceProcAddr(vkCreateDebugReportCallbackEXT);
		GetInstanceProcAddr(vkDebugReportMessageEXT);
		GetInstanceProcAddr(vkDestroyDebugReportCallbackEXT);

		VkDebugReportFlagsEXT flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

		VkDebugReportCallbackCreateInfoEXT drcCI{};
		drcCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		drcCI.flags = flags;
		drcCI.pfnCallback = &DebugReportCallback;
		m_vkCreateDebugReportCallbackEXT(m_instance, &drcCI, nullptr, &m_debugReport);
	}

}