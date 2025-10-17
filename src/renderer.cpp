#include "renderer.hpp"
#include <algorithm>
#include <map>
#include <set>
#include "util.hpp"

#ifndef NDEBUG
bool enableLogging = true;
#else
bool enableLogging = false;
#endif

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
	createSwapchainImageViews();
	createRenderPass();
	createGraphicsPipelineLayout();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
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


	if (checkInstanceExtensionSupport(requiredExtensions) && enableLogging)
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
	

	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS && enableLogging)
		mLogger.critical("Failed to create vulkan instance!");
	if(enableLogging)
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

		if (it == supportedExtensions.end() && enableLogging)
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

		if (it == supportedLayers.end() && enableLogging)
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

	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS && enableLogging)
		mLogger.critical("Failed to create a debug utils messenger!");
}

void ke::Renderer::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	if (deviceCount == 0 && enableLogging)
		mLogger.critical("No physical devices found on this machine.");

	std::map<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices)
	{
		unsigned int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0 && enableLogging)
	{
		mPhysicalDevice = candidates.rbegin()->second;
		mLogger.info(("Chosen physical device has score of " + std::to_string(candidates.rbegin()->first)));
	}
	else if(enableLogging)
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

	if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS && enableLogging)
		mLogger.critical("Failed to create logical device!");

	if(enableLogging)
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

	if (vkCreateWin32SurfaceKHR(mInstance, &createInfo, nullptr, &mSurface) != VK_SUCCESS && enableLogging)
		mLogger.error("Failed to create window surface.");

	if(enableLogging)
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
			if(enableLogging)
				mLogger.info("Desired format is available and chosen.");
			return format;
		}
	if(enableLogging)
		mLogger.warn("Desired format is not available.");
	return availableFormats[0];
}

VkPresentModeKHR ke::Renderer::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			if(enableLogging)
				mLogger.info("Mailbox present mode is available and chosen.");
			return presentMode;
		}
	if(enableLogging)
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

	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain) != VK_SUCCESS && enableLogging)
		mLogger.critical("Failed to create Swapchain!");
	if(enableLogging)
		mLogger.info("Created swapchain.");

	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
	mSwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

	mSwapchainImageFormat = surfaceFormat.format;
	mSwapchainExtent = extent;
}

void ke::Renderer::createSwapchainImageViews()
{
	mSwapchainImageViews.resize(mSwapchainImages.size());

	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.format = mSwapchainImageFormat;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	for (size_t i = 0; i < mSwapchainImages.size(); i++)
	{
		createInfo.image = mSwapchainImages[i];
		
		if (vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS && enableLogging)
			mLogger.error("Failed to create an image view.");
	}
	if(enableLogging)
		mLogger.info("Created swapchain image views.");
}

void ke::Renderer::createGraphicsPipelineLayout()
{
	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pushConstantRangeCount = 0;
	createInfo.setLayoutCount = 0;
	
	if (vkCreatePipelineLayout(mDevice, &createInfo, nullptr, &mPipelineLayout) != VK_SUCCESS && enableLogging)
		mLogger.error("Failed to create graphics pipeline layout!");
	if(enableLogging)
		mLogger.info("Created graphics pipeline layout.");
}

void ke::Renderer::createGraphicsPipeline()
{
	auto vertexCode = ke::util::readFile("shader/bin/vert.spv");
	auto fragCode = ke::util::readFile("shader/bin/frag.spv");

	auto vertexModule = createShaderModule(vertexCode);
	auto fragModule = createShaderModule(fragCode);

	VkPipelineShaderStageCreateInfo vertStage{};
	vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStage.module = vertexModule;
	vertStage.pName = "main";

	VkPipelineShaderStageCreateInfo fragStage{};
	fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStage.module = fragModule;
	fragStage.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };
	
	VkPipelineColorBlendAttachmentState colorAtt{};
	colorAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAtt.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlend{};
	colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlend.attachmentCount = 1;
	colorBlend.pAttachments = &colorAtt;
	colorBlend.logicOpEnable = VK_FALSE;

	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.sampleShadingEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport{};
	viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport.scissorCount = 1;
	viewport.viewportCount = 1;
	viewport.pViewports = nullptr;
	viewport.pScissors = nullptr;

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexAttributeDescriptionCount = 0;
	vertexInput.vertexBindingDescriptionCount = 0;

	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.layout = mPipelineLayout;
	createInfo.stageCount = 2;
	createInfo.pStages = shaderStages;
	createInfo.pColorBlendState = &colorBlend;
	createInfo.pDepthStencilState = nullptr;
	createInfo.pDynamicState = &dynamicState;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pMultisampleState = &multisampling;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pTessellationState = nullptr;
	createInfo.pViewportState = &viewport;
	createInfo.pVertexInputState = &vertexInput;
	createInfo.renderPass = mRenderPass;

	if (vkCreateGraphicsPipelines(mDevice, 0, 1, &createInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS && enableLogging)
		mLogger.critical("Failed to create a graphics pipeline!");
	if(enableLogging)
		mLogger.info("Created graphics pipeline!");

	vkDestroyShaderModule(mDevice, vertexModule, nullptr);
	vkDestroyShaderModule(mDevice, fragModule, nullptr);
}

void ke::Renderer::createRenderPass()
{
	VkAttachmentDescription colorAtt{};
	colorAtt.format = mSwapchainImageFormat;
	colorAtt.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAtt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorRef{};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &colorAtt;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(mDevice, &createInfo, nullptr, &mRenderPass) != VK_SUCCESS && enableLogging)
		mLogger.error("Failed to create render pass!");
	if(enableLogging)
		mLogger.info("Created render pass.");
}

void ke::Renderer::createCommandPool()
{
	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = indices.graphicsFamily.value();

	if (vkCreateCommandPool(mDevice, &createInfo, nullptr, &mCommandPool) != VK_SUCCESS && enableLogging)
		mLogger.error("Failed to create command pool!");
	if(enableLogging)
		mLogger.info("Created command pool.");
}

void ke::Renderer::createCommandBuffer()
{
	mCommandBuffers.resize(maxFramesInFlight);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;

	if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS && enableLogging)
		mLogger.critical("Failed to allocate command buffer!");
	if(enableLogging)
		mLogger.info("Created command buffer.");
}

void ke::Renderer::createFramebuffers()
{
	mFramebuffers.resize(mSwapchainImages.size());

	for (size_t i = 0; i < mSwapchainImages.size(); i++)
	{
		VkImageView attachment[] = { mSwapchainImageViews[i] };

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.height = mSwapchainExtent.height;
		createInfo.width = mSwapchainExtent.width;
		createInfo.layers = 1;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = attachment;
		createInfo.renderPass = mRenderPass;

		if (vkCreateFramebuffer(mDevice, &createInfo, nullptr, &mFramebuffers[i]) != VK_SUCCESS && enableLogging)
			mLogger.error("Framebuffer creation failed!");
		if(enableLogging)
			mLogger.info("Created framebuffer.");

	}
}

void ke::Renderer::createSyncObjects()
{
	mInFlightFences.resize(maxFramesInFlight);
	mImageReadySemaphores.resize(maxFramesInFlight);
	mRenderFinishedSemaphores.resize(maxFramesInFlight);
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		if (vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS ||
			vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageReadySemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS && enableLogging)
			mLogger.error("Failed to create at least one synchronisation object!");
	}
	
	if(enableLogging)
		mLogger.info("Created sync objects.");
}

VkShaderModule ke::Renderer::createShaderModule(const std::vector<char>& code) const
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule mod;
	if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &mod) != VK_SUCCESS && enableLogging)
		mLogger.error("Failed to create a shader module.");

	return mod;
}

void ke::Renderer::recreateSwapchain(GLFWwindow* pWindow)
{
	int width, height;
	glfwGetFramebufferSize(pWindow, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(pWindow, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(mDevice);

	cleanupSwapchain();

	createSwapchain(pWindow);
	createSwapchainImageViews();
	createFramebuffers();

}

void ke::Renderer::cleanupSwapchain()
{
	for (auto fb : mFramebuffers)
		vkDestroyFramebuffer(mDevice, fb, nullptr);
	for (auto imageView : mSwapchainImageViews)
		vkDestroyImageView(mDevice, imageView, nullptr);
	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
}

void ke::Renderer::cleanupRenderer()
{
	vkDeviceWaitIdle(mDevice);
	if(enableLogging)
	mLogger.trace("Initiating renderer cleanup.");

	cleanupSwapchain();

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		vkDestroySemaphore(mDevice, mImageReadySemaphores[i], nullptr);
		vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
	}
	
	vkFreeCommandBuffers(mDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	
	DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
	
	mLogger.trace("Renderer cleanup done.");
}

void ke::Renderer::beginRecording(GLFWwindow* pWindow, bool hasResized)
{
	vkWaitForFences(mDevice, 1, &mInFlightFences[currentFrameInFlight], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mImageReadySemaphores[currentFrameInFlight], VK_NULL_HANDLE, &currentImageIndex);


	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || hasResized)
	{
		framebufferResized = false;
		recreateSwapchain(pWindow);
		recreatedSwapchain = true;
	}
	else recreatedSwapchain = false;
		
	vkResetFences(mDevice, 1, &mInFlightFences[currentFrameInFlight]);

	vkResetCommandBuffer(mCommandBuffers[currentFrameInFlight], 0);

	VkCommandBufferBeginInfo cBeginInfo{};
	cBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	
	if (vkBeginCommandBuffer(mCommandBuffers[currentFrameInFlight], &cBeginInfo) != VK_SUCCESS)
		mLogger.critical("Failed to begin command buffer!");

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

	VkRenderPassBeginInfo rBeginInfo{};
	rBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rBeginInfo.clearValueCount = 1;
	rBeginInfo.pClearValues = &clearColor;
	rBeginInfo.framebuffer = mFramebuffers[currentImageIndex];
	rBeginInfo.renderArea.extent = mSwapchainExtent;
	rBeginInfo.renderArea.offset = { 0,0 };
	rBeginInfo.renderPass = mRenderPass;

	vkCmdBeginRenderPass(mCommandBuffers[currentFrameInFlight], &rBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(mCommandBuffers[currentFrameInFlight], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

	VkViewport viewport{};
	viewport.height = mSwapchainExtent.height;
	viewport.width = mSwapchainExtent.width;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(mCommandBuffers[currentFrameInFlight], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = mSwapchainExtent;
	scissor.offset = { 0,0 };
	vkCmdSetScissor(mCommandBuffers[currentFrameInFlight], 0, 1, &scissor);
}

void ke::Renderer::endRecording()
{
	if (recreatedSwapchain) return;

	vkCmdEndRenderPass(mCommandBuffers[currentFrameInFlight]);
	if (vkEndCommandBuffer(mCommandBuffers[currentFrameInFlight]) != VK_SUCCESS)
		mLogger.error("Failed to record command buffer!");

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphore[] = {mImageReadySemaphores[currentFrameInFlight]};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[currentFrameInFlight];
	VkSemaphore signalSemaphore[] = { mRenderFinishedSemaphores[currentFrameInFlight]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphore;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, mInFlightFences[currentFrameInFlight]) != VK_SUCCESS)
		mLogger.critical("Failed to submit to graphics queue!");

}

void ke::Renderer::present(GLFWwindow* pWindow)
{
	if (recreatedSwapchain) return;
	VkSemaphore waitSemaphore[] = { mRenderFinishedSemaphores[currentFrameInFlight]};

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = waitSemaphore;
	VkSwapchainKHR swapchains[] = { mSwapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pResults = nullptr;
	presentInfo.pImageIndices = &currentImageIndex;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		recreateSwapchain(pWindow);
}

void ke::Renderer::advanceFrame()
{
	currentFrameInFlight = (currentFrameInFlight + 1) % maxFramesInFlight;
}

VkCommandBuffer ke::Renderer::getCommandBuffer() const
{
	return mCommandBuffers[currentFrameInFlight];
}
