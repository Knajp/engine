#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "logger.hpp"
#include <vector>
#include <iostream>
#include <optional>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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
	std::optional<uint32_t> presentFamily;

	bool isComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
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

		void initVulkan(GLFWwindow* window);

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
		void createLogicalDevice();
		void createWindowSurface(GLFWwindow* window);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	private:
		VkInstance mInstance;

		VkDebugUtilsMessengerEXT mDebugMessenger;

		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		VkSurfaceKHR mSurface;
	private:
		ke::Logger mLogger = ke::Logger("Render Logger", spdlog::level::debug);
	};
}