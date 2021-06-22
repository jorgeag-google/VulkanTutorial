#pragma once

#include <iostream>
#include <string>
#include <vector>

/*
* With these define and include, we tell GLFW that we will use vulakn, so they will include 
* the vulkan header and loaders for us.
*/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Device.h"
/*
* This is the main class of the application. The execution starts at run
* which will initialize the app and then enter the main loop.
*/
class TriangleApp {
public:
	// This is where the app starts
	void run();
	// It needs to be public, since GLWF callbacks need to access it. And those need to be 
	// static and non-members
	bool mFramebufferResized{ false };
private:
	// App logic
	const uint32_t mWidth{800}; // window width in pixels
	const uint32_t mHeight{600}; // window height in pixels
	// To help with syncronization
	const int MAX_FRAMES_IN_FLIGHT{ 2 };
	// GLFW related
	GLFWwindow* mWindow{ nullptr }; // Window handle, so we can free it at the end of execution
	// Vulkan related
	VkInstance mInstance{};
	VkDebugUtilsMessengerEXT mDebugMessenger{}; // Handle to the debug logger callback
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	VkSurfaceKHR mSurface;
	VkRenderPass mRenderPass;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;
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
		"VK_LAYER_KHRONOS_validation" // Name of the validation layer to enable
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
	// Render
	void createSyncObjects();
	void drawFrame();
	// Presentation
	void createFramebuffers();
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
	// Factor out the filling of the debug message create info. Since we are going to use it in more than one place
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);