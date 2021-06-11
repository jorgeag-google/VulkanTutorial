#include <stdexcept>

#include "LoadModelApp.h"
#include "DebugLog.h"

void LoadModelApp::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void LoadModelApp::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mWindow = glfwCreateWindow(mWidth, mHeight, "Loading a model sample", nullptr, nullptr);
	// Register this pointer with GLFW (so we can use it in statci callbacks)
	glfwSetWindowUserPointer(mWindow, this);

	glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<LoadModelApp*>(glfwGetWindowUserPointer(window));
	app->mFramebufferResized = true;
}

void LoadModelApp::initVulkan() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createDepthResources();
	createFramebuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}

void LoadModelApp::mainLoop() {
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(mDevice);
}

void LoadModelApp::cleanup() {
	cleanupSwapChain();

	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
	}

	vkDestroySampler(mDevice, mTextureSampler, nullptr);
	vkDestroyImageView(mDevice, mTextureImageView, nullptr);
	vkDestroyImage(mDevice, mTextureImage, nullptr);
	vkFreeMemory(mDevice, mTextureImageMemory, nullptr);

	vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
	vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);
	vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
	vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);

	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	vkDestroyDevice(mDevice, nullptr);

	if (mEnableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);

	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void LoadModelApp::createInstance() {
	if (mEnableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// app Info struct help us configure the instance
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	// To pass it we need to wrap in in another create info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	// Validate that we have at least the required extensions
	// validateExtensions(extensions);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// If we want the validation layers request them
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (mEnableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Finally, we are ready to create the instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);
	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}
