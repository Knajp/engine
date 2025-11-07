#include "renderer.hpp"
#include <algorithm>
#include <map>
#include <set>
#include "stb/stb_image.h" 
#include <glm/gtc/matrix_transform.hpp>

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
	createDescriptorSetLayout();
	createGraphicsPipelineLayout();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createMenuTextureAtlas();
	createTextureSampler();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
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
	if (!deviceFeatures.samplerAnisotropy) return 0;

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
	deviceFeatures.samplerAnisotropy = VK_TRUE;

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

	for (size_t i = 0; i < mSwapchainImages.size(); i++)
	{
		mSwapchainImageViews[i] = createImageView(mSwapchainImages[i], mSwapchainImageFormat);
	}
	if(enableLogging)
		mLogger.info("Created swapchain image views.");
}

void ke::Renderer::createGraphicsPipelineLayout()
{
	VkPushConstantRange pcRange{};
	pcRange.offset = 0;
	pcRange.size = sizeof(ke::str::PushConstants);
	pcRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pushConstantRangeCount = 1;
	createInfo.pPushConstantRanges = &pcRange;
	createInfo.setLayoutCount = 1;
	createInfo.pSetLayouts = &mDescriptorSetLayout;
	
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

	auto bindingDesc = ke::str::Vertex::getInputBindingDescription();
	auto attribDescs = ke::str::Vertex::getInputAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescs.size());
	vertexInput.pVertexAttributeDescriptions = attribDescs.data();
	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.pVertexBindingDescriptions = &bindingDesc;

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
	while (width == 0 || height == 0 && !glfwWindowShouldClose(pWindow))
	{
		glfwGetFramebufferSize(pWindow, &width, &height);
		glfwWaitEventsTimeout(0.1);
	}

	if (width == 0 || height == 0) return;

	vkDeviceWaitIdle(mDevice);

	cleanupSwapchain();

	createSwapchain(pWindow);
	createSwapchainImageViews();
	createFramebuffers();

	recreateSemaphores();

	glfwFocusWindow(pWindow);

}

void ke::Renderer::cleanupSwapchain()
{
	for (auto fb : mFramebuffers)
		vkDestroyFramebuffer(mDevice, fb, nullptr);
	for (auto imageView : mSwapchainImageViews)
		vkDestroyImageView(mDevice, imageView, nullptr);
	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
}

void ke::Renderer::recreateSemaphores()
{
	for (auto sem : mImageReadySemaphores)
		vkDestroySemaphore(mDevice, sem, nullptr);

	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < maxFramesInFlight; i++)
		vkCreateSemaphore(mDevice, &createInfo, nullptr, &mImageReadySemaphores[i]);
}

void ke::Renderer::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding binding{};
	binding.descriptorCount = 1;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	binding.binding = 0;

	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.descriptorCount = 1;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerBinding.binding = 1;


	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { binding, samplerBinding };

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	createInfo.pBindings = bindings.data();
	
	if (vkCreateDescriptorSetLayout(mDevice, &createInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
		mLogger.error("Failed to create descriptor set layout!");
}

void ke::Renderer::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(ke::str::MVP);

	mUniformBuffers.resize(maxFramesInFlight);
	mUniformBufferMemories.resize(maxFramesInFlight);
	mUniformBuffersMapped.resize(maxFramesInFlight);

	ke::str::MVP ubo{};
	ubo.model = glm::mat4(1.0f);
	ubo.proj = glm::mat4(1.0f);
	ubo.view = glm::mat4(1.0f);

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i], mUniformBufferMemories[i]);

		vkMapMemory(mDevice, mUniformBufferMemories[i], 0, bufferSize, 0, &mUniformBuffersMapped[i]);


		memcpy(mUniformBuffersMapped[i], &ubo, sizeof(ke::str::MVP));
	}
}

void ke::Renderer::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();
	createInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight);

	if (vkCreateDescriptorPool(mDevice, &createInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
		mLogger.error("Failed to create descriptor pool!");
}

void ke::Renderer::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, mDescriptorSetLayout);
	
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pSetLayouts = layouts.data();
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
	
	mDescriptorSets.resize(maxFramesInFlight);
	if (vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS)
		mLogger.error("Failed to allocate descriptor sets!");

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(ke::str::MVP);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> writes{};
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = mDescriptorSets[i];
		writes[0].dstBinding = 0;
		writes[0].dstArrayElement = 0;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writes[0].pBufferInfo = &bufferInfo;

		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = mDescriptorSets[i];
		writes[1].dstBinding = 1;
		writes[1].dstArrayElement = 0;
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}
}

void ke::Renderer::updateUniformBuffer(float aspectRatio)
{
	ke::str::MVP mvp{};
	mvp.model = glm::mat4(1.0f);
	mvp.view = glm::mat4(1.0f);
	mvp.proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);

	memcpy(mUniformBuffersMapped[currentImageIndex], &mvp, sizeof(mvp));
}

void ke::Renderer::createTextureImage(VkImage& targetImage, VkDeviceMemory& targetMemory, std::string file)
{
	int tWidth, tHeight, numColCh;
	stbi_uc* pixels = stbi_load(file.c_str(), &tWidth, &tHeight, &numColCh, STBI_rgb_alpha);

	VkDeviceSize imageSize = tWidth * tHeight * 4;
	if (!pixels)
	{
		mLogger.error("Failed to load stb image!");
		return;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(mDevice, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(tWidth, tHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, targetImage, targetMemory);
	transitionImageLayout(targetImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, targetImage, tWidth, tHeight);
	transitionImageLayout(targetImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
		
}

void ke::Renderer::createTextureImageView(VkImageView& targetView, VkImage& sourceImage)
{
	targetView = createImageView(sourceImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void ke::Renderer::createFontAtlasImage(VkImage& targetImage, VkDeviceMemory& targetMemory, unsigned char* data)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(1024 * 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* bufferData;
	vkMapMemory(mDevice, stagingBufferMemory, 0, 1024 * 1024, 0, &bufferData);
	memcpy(bufferData, data, 1024 * 1024);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createImage(1024, 1024, VK_FORMAT_R8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, targetImage, targetMemory);
	transitionImageLayout(targetImage, VK_FORMAT_R8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, targetImage, 1024, 1024);
	transitionImageLayout(targetImage, VK_FORMAT_R8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void ke::Renderer::createFontAtlasView(VkImageView& targetView, VkImage& sourceImage)
{
	targetView = createImageView(sourceImage, VK_FORMAT_R8_SRGB);
}

void ke::Renderer::pushTexture(int16_t txt)
{
	ke::str::PushConstants pc = { txt };
	vkCmdPushConstants(mCommandBuffers[currentFrameInFlight], mPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ke::str::PushConstants), &pc);
}

void ke::Renderer::createMenuTextureAtlas()
{
	createTextureImage(textureImage, textureImageMemory, "txt/menu.png");
	createTextureImageView(textureView, textureImage);
}

void ke::Renderer::createTextureSampler()
{
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.minFilter = VK_FILTER_NEAREST;
	createInfo.magFilter = VK_FILTER_NEAREST;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties prop{};
	vkGetPhysicalDeviceProperties(mPhysicalDevice, &prop);

	createInfo.maxAnisotropy = prop.limits.maxSamplerAnisotropy;
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

	if (vkCreateSampler(mDevice, &createInfo, nullptr, &textureSampler) != VK_SUCCESS)
		mLogger.error("Failed to create texture sampler!");

}

void ke::Renderer::createImage(int width, int height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.extent.width = static_cast<uint32_t>(width);
	createInfo.extent.height = static_cast<uint32_t>(height);
	createInfo.extent.depth = 1;
	createInfo.arrayLayers = 1;
	createInfo.mipLevels = 1;
	createInfo.format = format;
	createInfo.tiling = tiling;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	if (vkCreateImage(mDevice, &createInfo, nullptr, &image) != VK_SUCCESS)
		mLogger.error("Failed to create image!");

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(mDevice, image, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		mLogger.error("Failed to allocate image memory!");

	vkBindImageMemory(mDevice, image, imageMemory, 0);
}

VkImageView ke::Renderer::createImageView(VkImage image, VkFormat format)
{
	VkImageView imageView;

	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;

	if (vkCreateImageView(mDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		mLogger.error("Failed to create image view!");

	return imageView;
}

void ke::Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
	VkCommandBuffer commandBuffer = beginSingeTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = srcLayout;
	barrier.newLayout = dstLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	
	if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED && dstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.srcAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && dstLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else mLogger.warn("Unsupported image layout transition.");

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(commandBuffer);
}

void ke::Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingeTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferImageHeight = 0;
	region.bufferRowLength = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageOffset = { 0,0,0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer ke::Renderer::beginSingeTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	
	VkCommandBuffer buffer;
	if (vkAllocateCommandBuffers(mDevice, &allocInfo, &buffer) != VK_SUCCESS)
		mLogger.error("Failed to allocate single time command buffer!");

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	
	vkBeginCommandBuffer(buffer, &beginInfo);

	return buffer;
}

void ke::Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

void ke::Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags flags, VkMemoryPropertyFlags memoryFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = flags;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(getInstance().mDevice, &createInfo, nullptr, &buffer) != VK_SUCCESS && enableLogging)
		mLogger.error("Failed to create a buffer.");
	if (enableLogging)
		mLogger.info("Created buffer.");

	VkMemoryRequirements memReq{};
	vkGetBufferMemoryRequirements(mDevice, buffer, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, memoryFlags);

	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		mLogger.error("Failed to allocate vertex buffer memory.");

	vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
}

void ke::Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingeTimeCommands();
	
	VkBufferCopy copy{};
	copy.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copy);

	endSingleTimeCommands(commandBuffer);
}

uint32_t ke::Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties memProp;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProp);

	for (uint32_t i = 0; i < memProp.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProp.memoryTypes[i].propertyFlags & flags) == flags)
			return i;
	}

	mLogger.error("Failed to find a suitable memory type.");

	return -1;
}

void ke::Renderer::cleanupRenderer()
{
	vkDeviceWaitIdle(mDevice);
	if(enableLogging)
	mLogger.trace("Initiating renderer cleanup.");

	destroyRedundantBuffers();
	cleanupSwapchain();

	vkDestroyImage(mDevice, textureImage, nullptr);
	vkDestroyImageView(mDevice, textureView, nullptr);
	vkFreeMemory(mDevice, textureImageMemory, nullptr);
	vkDestroySampler(mDevice, textureSampler, nullptr);

	for (const auto buffer : mUniformBuffers)
	{
		vkDestroyBuffer(mDevice, buffer, nullptr);
	}
	for (const auto memory : mUniformBufferMemories)
		vkFreeMemory(mDevice, memory, nullptr);
	vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
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
	vkDestroyDevice(mDevice, nullptr);
	vkDestroyInstance(mInstance, nullptr);
	
	mLogger.trace("Renderer cleanup done.");
}

void ke::Renderer::beginRecording(GLFWwindow* pWindow, bool hasResized, float ar)
{
	vkWaitForFences(mDevice, 1, &mInFlightFences[currentFrameInFlight], VK_TRUE, UINT64_MAX);
	VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mImageReadySemaphores[currentFrameInFlight], VK_NULL_HANDLE, &currentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || hasResized)
	{
		framebufferResized = false;
		recreateSwapchain(pWindow);
		for (size_t i = 0; i < mCommandBuffers.size(); i++) {
			vkResetCommandBuffer(mCommandBuffers[i], 0);
		}
		recreatedSwapchain = true;
		return;
	}
	else recreatedSwapchain = false;
		
	updateUniformBuffer(ar);

	vkResetFences(mDevice, 1, &mInFlightFences[currentFrameInFlight]);

	vkResetCommandBuffer(mCommandBuffers[currentFrameInFlight], 0);

	VkCommandBufferBeginInfo cBeginInfo{};
	cBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	
	if (vkBeginCommandBuffer(mCommandBuffers[currentFrameInFlight], &cBeginInfo) != VK_SUCCESS)
		mLogger.critical("Failed to begin command buffer!");

	vkCmdBindPipeline(mCommandBuffers[currentFrameInFlight], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

	VkViewport viewport{};
	viewport.height = static_cast<float>(mSwapchainExtent.height);
	viewport.width = static_cast<float>(mSwapchainExtent.width);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0;
	viewport.y = 0;

	VkRect2D scissor{};
	scissor.extent = mSwapchainExtent;
	scissor.offset = { 0,0 };

	vkCmdSetViewport(mCommandBuffers[currentFrameInFlight], 0, 1, &viewport);
	vkCmdSetScissor(mCommandBuffers[currentFrameInFlight], 0, 1, &scissor);

	vkCmdBindDescriptorSets(mCommandBuffers[currentFrameInFlight], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[currentFrameInFlight], 0, nullptr);


	VkClearValue clearColor = { {{0.067f, 0.067f, 0.067f, 1.0f}} };

	VkRenderPassBeginInfo rBeginInfo{};
	rBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rBeginInfo.clearValueCount = 1;
	rBeginInfo.pClearValues = &clearColor;
	rBeginInfo.framebuffer = mFramebuffers[currentImageIndex];
	rBeginInfo.renderArea.extent = mSwapchainExtent;
	rBeginInfo.renderArea.offset = { 0,0 };
	rBeginInfo.renderPass = mRenderPass;

	vkCmdBeginRenderPass(mCommandBuffers[currentFrameInFlight], &rBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
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

VkCommandBuffer& ke::Renderer::getCommandBuffer()
{
	return mCommandBuffers[currentFrameInFlight];
}

VkDevice ke::Renderer::getDevice()
{
	return ke::Renderer::getInstance().mDevice;
}

void ke::Renderer::createVertexBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const std::vector<ke::str::Vertex>& vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	getInstance().createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(getInstance().mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(getInstance().mDevice, stagingBufferMemory);

	getInstance().createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
	getInstance().copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(ke::Renderer::getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(ke::Renderer::getDevice(), stagingBufferMemory, nullptr);

}

void ke::Renderer::createIndexBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const std::vector<uint16_t>& indices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	getInstance().createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(getInstance().mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(getInstance().mDevice, stagingBufferMemory);

	getInstance().createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
	getInstance().copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(ke::Renderer::getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(ke::Renderer::getDevice(), stagingBufferMemory, nullptr);
}

void ke::Renderer::submitBufferForDestruction(std::pair<VkBuffer, VkDeviceMemory> buffer)
{
	mDestroyVector.push_back(buffer);
}

void ke::Renderer::destroyRedundantBuffers()
{
	if (mDestroyVector.empty()) return;

	vkDeviceWaitIdle(mDevice);

	for (const auto& buffer : mDestroyVector)
	{
		vkDestroyBuffer(mDevice, buffer.first, nullptr);
		vkFreeMemory(mDevice, buffer.second, nullptr);
	}
	
	mDestroyVector.clear();
	mDestroyVector.resize(0);
}

bool ke::Renderer::hasRecreatedSwapchain() const
{
	return recreatedSwapchain;
}
