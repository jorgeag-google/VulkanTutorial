#pragma once

#include <iostream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Device.h"

class VertexBufferApp {
public:
	void run();
	bool mFramebufferResized{ false };
private:
	// App logic
	const uint32_t mWidth{ 800 };
	const uint32_t mHeight{ 600 };
	// To help with syncronization
	const int MAX_FRAMES_IN_FLIGHT{ 2 };
	// GLFW related
	GLFWwindow* mWindow{ nullptr };
	// Vulkan related
	VkInstance mInstance{};
	VkDebugUtilsMessengerEXT mDebugMessenger{};
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	VkSurfaceKHR mSurface;
	VkRenderPass mRenderPass;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
	std::vector<VkFramebuffer> mSwapChainFramebuffers;
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;
	// Vulkan's swapchain related
	VkSwapchainKHR mSwapChain;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mSwapChainImageViews;
	// Syncronization
	std::vector <VkSemaphore> mImageAvailableSemaphores;
	std::vector <VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;
	std::vector<VkFence> mImagesInFlight;
	size_t mCurrentFrame{ 0 };
	// Device related
	VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };
	VkDevice mDevice{ VK_NULL_HANDLE };
	// Enable validation layers and debug
	const std::vector<const char*> mValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	// List of extension required for the device to be suitable
	const std::vector<const char*> mDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
#ifdef NDEBUG
	const bool mEnableValidationLayers = false;
#else
	const bool mEnableValidationLayers = true;
#endif
	// General program flow functions
	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();
	// Prepare the render target functions
	void createSurface();
	void createSwapChain();
	void recreateSwapChain();
	void cleanupSwapChain();
	void createImageViews();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	// Pipeline functions
	void createGraphicsPipeline();
	void createRenderPass();
	// Comand recording
	void createCommandPool();
	void createCommandBuffers();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	// Render
	void createSyncObjects();
	void drawFrame();
	// Buffere management
	void createVertexBuffer();
	void createIndexBuffer();
	void createFramebuffers();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	// Vulkan initialization functions
	void createInstance();
	std::vector<const char*> getRequiredExtensions();
	void validateExtensions(const std::vector<const char*>& requiredExt);
	// Device manage related
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	// Validation layer and debug logger support
	void setupDebugMessenger();
	bool checkValidationLayerSupport();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);