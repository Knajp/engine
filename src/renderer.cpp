#include "renderer.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

const std::vector<const char*> gValidationLayers
{
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
bool enableValidationLayers = false;
#else
bool enableValidationLayers = true;
#endif

void ke::Renderer::initVulkan()
{
	createVulkanInstance();
}

void ke::Renderer::createVulkanInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "Knaj's engine";
	appInfo.pEngineName = "No engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	std::vector<const char*> requiredExtensions = getRequiredExtensions();

	if (checkInstanceExtensionSupport(requiredExtensions))
		mLogger.info("All required instance extensions are supported.");

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	if (enableValidationLayers)
	{
		mLogger.debug("Requested validation layers.");
		createInfo.enabledLayerCount = static_cast<uint32_t>(gValidationLayers.size());
		createInfo.ppEnabledLayerNames = gValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}
	

	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
		mLogger.critical("Failed to create vulkan instance!");
	mLogger.info("Created vulkan instance.");
}

bool ke::Renderer::checkInstanceExtensionSupport(const std::vector<const char*>& exts)
{
	uint32_t supportedExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
	std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

	bool failedCheck = false;

	for (const auto& ext : exts)
	{
		auto it = std::find_if(supportedExtensions.begin(), supportedExtensions.end(),
			[ext](const VkExtensionProperties& prop)
			{
				return std::strcmp(prop.extensionName, ext) == 0;
			});

		if (it == supportedExtensions.end())
		{
			mLogger.error("A required instance extension was not found in the supported extension array.");
			failedCheck = true;
		}
	}

	return !failedCheck;
}

bool ke::Renderer::checkValidationLayerSupport()
{
	uint32_t supportedLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
	std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
	vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());

	bool failedCheck = false;

	for (const auto& layer : gValidationLayers)
	{
		auto it = std::find_if(supportedLayers.begin(), supportedLayers.end(),
			[layer](const VkLayerProperties& prop)
			{
				return std::strcmp(prop.layerName, layer) == 0;
			});

		if (it == supportedLayers.end())
		{
			mLogger.error("A validation layer is not supported.");
			failedCheck = true;
		}
	}

	return !failedCheck;
}

std::vector<const char*> ke::Renderer::getRequiredExtensions()
{
	std::vector<const char*> requiredExtensions;
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

	if (enableValidationLayers)
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return requiredExtensions;
}

void ke::Renderer::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;

	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
		mLogger.critical("Failed to create a debug utils messenger!");
}

void ke::Renderer::cleanupRenderer()
{
	mLogger.trace("Initiating renderer cleanup.");

	DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	vkDestroyInstance(mInstance, nullptr);
	
	mLogger.trace("Renderer cleanup done.");
}
