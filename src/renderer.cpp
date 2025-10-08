#include "renderer.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <map>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugUtilsMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugUtilsMessenger);
	else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);

}

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
	setupDebugMessenger();
	pickPhysicalDevice();
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

void ke::Renderer::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	if (deviceCount == 0)
		mLogger.critical("No physical devices found on this machine.");

	std::map<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices)
	{
		unsigned int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0)
	{
		mPhysicalDevice = candidates.rbegin()->second;
		mLogger.info(("Chosen physical device has score of " + std::to_string(candidates.rbegin()->first)));
	}
	else
		mLogger.critical("Failed to find a suitable physical device!");
}

unsigned int ke::Renderer::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties  deviceProperties{};
	VkPhysicalDeviceFeatures deviceFeatures{};
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

	score += deviceProperties.limits.maxImageDimension2D;

	if (!deviceFeatures.geometryShader) return 0;

	QueueFamilyIndices indices = findQueueFamilies(device);
	if (!indices.isComplete()) return 0;

	return score;
}

QueueFamilyIndices ke::Renderer::findQueueFamilies(VkPhysicalDevice device) const
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& family : queueFamilies)
	{
		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;


		if (indices.isComplete()) break;
		i++;
	}

	return indices;
}

void ke::Renderer::cleanupRenderer()
{
	mLogger.trace("Initiating renderer cleanup.");

	DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	vkDestroyInstance(mInstance, nullptr);
	
	mLogger.trace("Renderer cleanup done.");
}
