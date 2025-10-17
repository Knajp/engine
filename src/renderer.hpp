#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
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

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

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

		void beginRecording(GLFWwindow* pWindow, bool hasResized);
		void endRecording();
		void present(GLFWwindow* pWindow);

		void advanceFrame();
		VkCommandBuffer getCommandBuffer() const;
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
		inline SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) const;
		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* pWindow);
		void createSwapchain(GLFWwindow* pWindow);
		void createSwapchainImageViews();
		void createGraphicsPipelineLayout();
		void createGraphicsPipeline();
		void createRenderPass();
		void createCommandPool();
		void createCommandBuffer();
		void createFramebuffers();
		void createSyncObjects();
		VkShaderModule createShaderModule(const std::vector<char>& code) const;
		void recreateSwapchain(GLFWwindow* pWindow);
		void cleanupSwapchain();
	private:
		VkInstance mInstance;

		VkDebugUtilsMessengerEXT mDebugMessenger;

		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mDevice = VK_NULL_HANDLE;

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		VkSurfaceKHR mSurface;

		VkSwapchainKHR mSwapchain;
		std::vector<VkImage> mSwapchainImages;
		std::vector<VkImageView> mSwapchainImageViews;

		VkFormat mSwapchainImageFormat;
		VkExtent2D mSwapchainExtent;
		
		VkRenderPass mRenderPass;

		VkPipelineLayout mPipelineLayout;
		VkPipeline mGraphicsPipeline;

		VkCommandPool mCommandPool;
		std::vector<VkCommandBuffer> mCommandBuffers;

		std::vector<VkFramebuffer> mFramebuffers;

		std::vector<VkFence> mInFlightFences;
		std::vector<VkSemaphore> mImageReadySemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;

		uint32_t currentImageIndex;
		uint32_t currentFrameInFlight = 0;
		const uint8_t maxFramesInFlight = 2;

		bool recreatedSwapchain = false;
		bool framebufferResized = false;
	private:
		ke::Logger mLogger = ke::Logger("Render Logger", spdlog::level::debug);
	};
}