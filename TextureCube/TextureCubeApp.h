#pragma once

#include <iostream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Trackball.h"
#include "Vertex.h"
#include "Device.h"

class TextureCubeApp {
public:
	void run();
	bool mFramebufferResized{ false };
	bool mRotate{ true };
	// Camera related
	Trackball mTrackball;
	bool mMouseDrag{ false };      //To keep track the mouse drag for camera rotation
	int mZoomLevel{ 0 };       //Zoom level to complement the trackball camera
private:
	// App logic
	const uint32_t mWidth{ 800 };
	const uint32_t mHeight{ 600 };
	const std::string MODEL_PATH{ "models/viking_room.obj" };
	const std::string TEXTURE_PATH{ "textures/viking_room.png" };
	// Model loading
	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;
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
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;
	std::vector<VkFramebuffer> mSwapChainFramebuffers;
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;
	// Vulkan's swapchain related
	VkSwapchainKHR mSwapChain;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mSwapChainImageViews;
	// Multisample
	VkSampleCountFlagBits mMsaaSamples{ VK_SAMPLE_COUNT_1_BIT };
	VkImage mColorImage;
	VkDeviceMemory mColorImageMemory;
	VkImageView mColorImageView;
	// Texture image
	// For specular texture
	uint32_t mSpecTextMipLevels;
	VkImage mSpecularTextureImage;
	VkDeviceMemory mSpecularTextureImageMemory;
	VkImageView mSpecularTextureImageView;
	VkSampler mSpecularTextureSampler;
	// For diffuse texture
	uint32_t mDiffTextMipLevels;
	VkImage mDiffuseTextureImage;
	VkDeviceMemory mDiffuseTextureImageMemory;
	VkImageView mDiffuseTextureImageView;
	VkSampler mDiffuseTextureSampler;
	// Depth buffer
	VkImage mDepthImage;
	VkDeviceMemory mDepthImageMemory;
	VkImageView mDepthImageView;
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
	// Model loading
	void loadModel();
	void loadModelFromFile(const std::string fileName = "");
	void loadTextureCube();
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
	void createDescriptorPool();
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
	void createUniformBuffers();
	void updateUniformBuffer(uint32_t currentImage);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	// Uniforms management
	void createDescriptorSetLayout();
	void createDescriptorSets();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	// Multisample
	VkSampleCountFlagBits getMaxUsableSampleCount();
	void createColorResources();
	// Depth buffer
	void createDepthResources();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
		VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	// Texture related
	void createTextureImages();
	void createTextureFromFile(const std::string fileName, VkImage& image, 
		VkDeviceMemory& imageMemory, uint32_t& mipLevels);
	void createTextureImageViews();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
		uint32_t mipLevels);
	void createTextureSamplers();
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
		VkDeviceMemory& imageMemory);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
		VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
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
static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, 
		int mods);
static void cursorPositionCallback(GLFWwindow* window, double mouseX, double mouseY);
static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);