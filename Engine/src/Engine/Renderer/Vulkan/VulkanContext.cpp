#include "epch.h"
#include "VulkanContext.h"

#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>
#include <GLFW/glfw3.h>

constexpr uint32_t USE_VULKAN_VERSION = VK_API_VERSION_1_2;

namespace {

#ifdef DEBUG
	static bool s_Validation = true;
#else
	static bool s_Validation = false;
#endif

	bool CheckDriverAPIVersionSupport(uint32_t minimumSupportedVersion)
	{
		uint32_t instanceVersion;
		vkEnumerateInstanceVersion(&instanceVersion);

		if (instanceVersion < minimumSupportedVersion)
		{
			LOG_ENGINE_FATAL("Incompatible Vulkan driver version!");
			//LOG_ENGINE_FATAL("  You have {}.{}.{}", VK_API_VERSION_MAJOR(instanceVersion), VK_API_VERSION_MINOR(instanceVersion), VK_API_VERSION_PATCH(instanceVersion));
			//LOG_ENGINE_FATAL("  You need at least {}.{}.{}", VK_API_VERSION_MAJOR(minimumSupportedVersion), VK_API_VERSION_MINOR(minimumSupportedVersion), VK_API_VERSION_PATCH(minimumSupportedVersion));

			return false;
		}

		return true;
	}

	inline void DumpGPUInfo()
	{
#if 0
		auto& caps = Renderer::GetCapabilities();
		HZ_CORE_TRACE_TAG("Renderer", "GPU Info:");
		HZ_CORE_TRACE_TAG("Renderer", "  Vendor: {0}", caps.Vendor);
		HZ_CORE_TRACE_TAG("Renderer", "  Device: {0}", caps.Device);
		HZ_CORE_TRACE_TAG("Renderer", "  Version: {0}", caps.Version);
#endif
	}

	inline const char* VKResultToString(VkResult result)
	{
		switch (result)
		{
		case VK_SUCCESS: return "VK_SUCCESS";
		case VK_NOT_READY: return "VK_NOT_READY";
		case VK_TIMEOUT: return "VK_TIMEOUT";
		case VK_EVENT_SET: return "VK_EVENT_SET";
		case VK_EVENT_RESET: return "VK_EVENT_RESET";
		case VK_INCOMPLETE: return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_PIPELINE_COMPILE_REQUIRED_EXT: return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
		}
		ENGINE_ASSERT(false);
		return nullptr;
	}

	inline void VulkanCheckResult(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			LOG_ENGINE_ERROR("VkResult is '{0}' in {1}:{2}", VKResultToString(result), __FILE__, __LINE__);
			if (result == VK_ERROR_DEVICE_LOST)
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(3s);
				DumpGPUInfo();
			}
			ENGINE_ASSERT(result == VK_SUCCESS);
		}
	}

	PFN_vkSetDebugUtilsObjectNameEXT fpSetDebugUtilsObjectNameEXT; //Making it static randomly sets it to nullptr for some reason.
	PFN_vkCmdBeginDebugUtilsLabelEXT fpCmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT fpCmdEndDebugUtilsLabelEXT;
	PFN_vkCmdInsertDebugUtilsLabelEXT fpCmdInsertDebugUtilsLabelEXT;
	void VulkanLoadDebugUtilsExtensions(VkInstance instance)
	{
		fpSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
		if (fpSetDebugUtilsObjectNameEXT == nullptr)
			fpSetDebugUtilsObjectNameEXT = [](VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) { return VK_SUCCESS; };

		fpCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
		if (fpCmdBeginDebugUtilsLabelEXT == nullptr)
			fpCmdBeginDebugUtilsLabelEXT = [](VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {};

		fpCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
		if (fpCmdEndDebugUtilsLabelEXT == nullptr)
			fpCmdEndDebugUtilsLabelEXT = [](VkCommandBuffer commandBuffer) {};

		fpCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT"));
		if (fpCmdInsertDebugUtilsLabelEXT == nullptr)
			fpCmdInsertDebugUtilsLabelEXT = [](VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {};
	}

}

#define VK_CHECK_RESULT(f)\
{\
	VkResult res = (f);\
	::VulkanCheckResult(res);\
}

namespace Engine {

	VulkanContext::~VulkanContext()
	{


		vkDestroyInstance(s_VulkanInstance, nullptr);
		s_VulkanInstance = nullptr;
	}

	void VulkanContext::Init()
	{
		LOG_ENGINE_INFO_TAG("Renderer", "VulkanContext::Create");
		ENGINE_ASSERT(glfwVulkanSupported(), "GLFW must support Vulkan!");

		if (!CheckDriverAPIVersionSupport(USE_VULKAN_VERSION))
		{
			MessageBox(nullptr, L"Incompatible Vulkan driver version.\nUpdate your GPU drivers!", L"Hazel Error", MB_OK | MB_ICONERROR);
			ENGINE_VERIFY(false);
		}

#pragma region Application Info
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Engine";
		appInfo.pEngineName = "Engine";
		appInfo.apiVersion = USE_VULKAN_VERSION;
#pragma endregion

#pragma region Extensions and Validation
#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
		std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Very little performance hit, can be used in Release.
		if (s_Validation)
		{
			instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
		VkValidationFeaturesEXT features = {};
		features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		features.enabledValidationFeatureCount = 1;
		features.pEnabledValidationFeatures = enables;

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr; // &features;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

		// TODO: Extract all validation into separate class
		if (s_Validation)
		{
			const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
			// Check if this layer is available at instance level
			uint32_t instanceLayerCount;
			vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
			std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
			vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
			bool validationLayerPresent = false;
			LOG_ENGINE_TRACE_TAG("Renderer", "Vulkan Instance Layers:");
			for (const VkLayerProperties& layer : instanceLayerProperties)
			{
				LOG_ENGINE_TRACE_TAG("Renderer", "  {0}", layer.layerName);
				if (strcmp(layer.layerName, validationLayerName) == 0)
				{
					validationLayerPresent = true;
					break;
				}
			}
			if (validationLayerPresent)
			{
				instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
				instanceCreateInfo.enabledLayerCount = 1;
			}
			else
			{
				LOG_ENGINE_ERROR_TAG("Renderer", "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled");
			}
		}
#pragma endregion

#pragma region Instance and Surface Creation
		VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &s_VulkanInstance));
		VulkanLoadDebugUtilsExtensions(s_VulkanInstance);

		//if (s_Validation)
		//{
		//	auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_VulkanInstance, "vkCreateDebugUtilsMessengerEXT");
		//	HZ_CORE_ASSERT(vkCreateDebugUtilsMessengerEXT != NULL, "");
		//	VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
		//	debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		//	debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//	debugUtilsCreateInfo.pfnUserCallback = VulkanDebugUtilsMessengerCallback;
		//	debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT /*  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		//		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT*/;

		//	VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(s_VulkanInstance, &debugUtilsCreateInfo, nullptr, &m_DebugUtilsMessenger));
		//}
#pragma endregion

	}

}