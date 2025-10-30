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
#include "util.hpp"


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

		static VkDevice getDevice();
		static void createVertexBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const std::vector<ke::str::Vertex>& vertices);
		static void createIndexBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const std::vector<uint16_t>& indices);

		void submitBufferForDestruction(std::pair<VkBuffer, VkDeviceMemory> buffer);
		void destroyRedundantBuffers();

		bool hasRecreatedSwapchain() const;

		void createTextureImage(VkImage& targetImage, VkDeviceMemory& targetMemory);
		void createTextureImageView(VkImageView& targetView, VkImage& sourceImage);
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
		void recreateSemaphores();
		void createDescriptorSetLayout();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		
		
		void createTextureSampler();

		void createImage(int width, int height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout srcLayout, VkImageLayout dstLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		VkCommandBuffer beginSingeTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags flags, VkMemoryPropertyFlags memoryFlags, VkBuffer& buffer, VkDeviceMemory& memory);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags);

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

		VkDescriptorSetLayout mDescriptorSetLayout;
		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBufferMemories;
		std::vector<void*> mUniformBuffersMapped;
		VkDescriptorPool mDescriptorPool;
		std::vector<VkDescriptorSet> mDescriptorSets;

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;

	private:
		ke::Logger mLogger = ke::Logger("Render Logger", spdlog::level::debug);
		std::vector<std::pair<VkBuffer, VkDeviceMemory>> mDestroyVector;
	};
}