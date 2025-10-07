#pragma once
#include <vulkan/vulkan.h>
#include "logger.hpp"
#include <vector>
#include <iostream>

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
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		std::cerr << "Validaton layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

namespace ke
{
	class Renderer
	{
	public:
		static Renderer& getInstance()
		{
			static Renderer instance;
			return instance;
		}

		Renderer(Renderer& other) = delete;
		Renderer operator=(Renderer& other) = delete;

		void initVulkan();

		void cleanupRenderer();
	private:
		Renderer() = default;

		void createVulkanInstance();
		bool checkInstanceExtensionSupport(const std::vector<const char*>& exts);
		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions();
		void setupDebugMessenger();
	private:
		VkInstance mInstance;

		VkDebugUtilsMessengerEXT mDebugMessenger;
	private:
		ke::Logger mLogger = ke::Logger("Render Logger", spdlog::level::debug);
	};
}