#pragma once
#include <vulkan/vulkan.h>
#include "logger.hpp"
#include <vector>
#include <iostream>
#include <optional>

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

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() const
	{
		return graphicsFamily.has_value();
	}
};

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
		void pickPhysicalDevice();
		unsigned int rateDeviceSuitability(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
	private:
		VkInstance mInstance;

		VkDebugUtilsMessengerEXT mDebugMessenger;

		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		
	private:
		ke::Logger mLogger = ke::Logger("Render Logger", spdlog::level::debug);
	};
}