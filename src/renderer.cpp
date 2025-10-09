#include "renderer.hpp"
#include <algorithm>
#include <map>
#include <set>
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugUtilsMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugUtilsMessenger);
	else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);

}

const std::vector<const char*> gValidationLayers
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> gDeviceExtensions
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
bool enableValidationLayers = false;
#else
bool enableValidationLayers = true;
#endif

void ke::Renderer::initVulkan(GLFWwindow* window)
{
	createVulkanInstance();
	setupDebugMessenger();
	createWindowSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapchain(window);
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
	requiredExtensions.push_back("VK_KHR_win32_surface");
	requiredExtensions.push_back("VK_KHR_surface");

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

	if (!checkDeviceExtensionSupport(device)) return 0;

	SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
	if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty()) return 0;

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

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

		if (presentSupport)
			indices.presentFamily = i;

		if (indices.isComplete()) break;
		i++;
	}

	return indices;
}

void ke::Renderer::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

	float queuePriority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> createInfos;
	std::set<uint32_t> uniqueQueueIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	
	for (const auto& index : uniqueQueueIndices)
	{
		VkDeviceQueueCreateInfo graphicsQueueInfo{};
		graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueInfo.queueCount = 1;
		graphicsQueueInfo.queueFamilyIndex = index;
		graphicsQueueInfo.pQueuePriorities = &queuePriority;

		createInfos.push_back(graphicsQueueInfo);
	}
	

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(createInfos.size());
	createInfo.pQueueCreateInfos = createInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(gDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = gDeviceExtensions.data();
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(gValidationLayers.size());
		createInfo.ppEnabledLayerNames = gValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
		mLogger.critical("Failed to create logical device!");

	mLogger.info("Created a logical device.");

	vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &presentQueue);
}

void ke::Renderer::createWindowSurface(GLFWwindow* window)
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(window);
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(mInstance, &createInfo, nullptr, &mSurface) != VK_SUCCESS)
		mLogger.error("Failed to create window surface.");

	mLogger.info("Created a window surface.");
}

bool ke::Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
	
	std::set<std::string> requiredExtensions(gDeviceExtensions.begin(), gDeviceExtensions.end());

	for (const auto& extension : extensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
		
}

SwapchainSupportDetails ke::Renderer::querySwapchainSupport(VkPhysicalDevice device) const
{
	SwapchainSupportDetails supportDetails;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &supportDetails.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		supportDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, supportDetails.formats.data());
	}
	
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		supportDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, supportDetails.presentModes.data());
	}



	return supportDetails;
}

VkSurfaceFormatKHR ke::Renderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats)
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			mLogger.info("Desired format is available and chosen.");
			return format;
		}
	mLogger.warn("Desired format is not available.");
	return availableFormats[0];
}

VkPresentModeKHR ke::Renderer::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			mLogger.info("Mailbox present mode is available and chosen.");
			return presentMode;
		}
	mLogger.warn("Mailbox present mode is not available, defaulting to FIFO.");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ke::Renderer::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* pWindow)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	int width, height;
	glfwGetFramebufferSize(pWindow, &width, &height);

	VkExtent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

void ke::Renderer::createSwapchain(GLFWwindow* pWindow)
{
	SwapchainSupportDetails supportDetails = querySwapchainSupport(mPhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(supportDetails.formats);
	VkPresentModeKHR presentMode = chooseSurfacePresentMode(supportDetails.presentModes);
	VkExtent2D extent = chooseSwapchainExtent(supportDetails.capabilities, pWindow);

	uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
	if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount)
		imageCount = supportDetails.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.presentMode = presentMode;
	createInfo.surface = mSurface;
	createInfo.minImageCount = supportDetails.capabilities.minImageCount;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	createInfo.preTransform = supportDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
		mLogger.critical("Failed to create Swapchain!");
	mLogger.info("Created swapchain.");

	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
	mSwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

	mSwapchainImageFormat = surfaceFormat.format;
	mSwapchainExtent = extent;
}

void ke::Renderer::cleanupRenderer()
{
	mLogger.trace("Initiating renderer cleanup.");

	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
	vkDestroyDevice(mDevice, nullptr);
	DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
	
	mLogger.trace("Renderer cleanup done.");
}
