#include <stdexcept>

#include "TextureCubeApp.h"
#include "DebugLog.h"

void TextureCubeApp::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void TextureCubeApp::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mWindow = glfwCreateWindow(mWidth, mHeight, "Textured cube in Vulkan", nullptr, nullptr);
	mTrackball = Trackball(mWidth, mHeight);
	// Register this pointer with GLFW (so we can use it in static callbacks)
	glfwSetWindowUserPointer(mWindow, this);

	glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
	glfwSetKeyCallback(mWindow, keyboardCallback);
	glfwSetCursorPosCallback(mWindow, cursorPositionCallback);
	glfwSetScrollCallback(mWindow, scrollCallback);
	glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
}

static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto app = reinterpret_cast<TextureCubeApp*>(glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, 1);
				break;
			case GLFW_KEY_R: {
				app->mRotate = !app->mRotate;
			}
		}
	}
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<TextureCubeApp*>(glfwGetWindowUserPointer(window));
	app->mFramebufferResized = true;
	app->mTrackball.setWindowSize(width, height);
}

static void cursorPositionCallback(GLFWwindow* window, double mouseX, double mouseY) {
	auto app = reinterpret_cast<TextureCubeApp*>(glfwGetWindowUserPointer(window));
	if (app->mMouseDrag) {
		app->mTrackball.drag(glm::ivec2(int(mouseX), int(mouseY)));
	}
}

static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	auto app = reinterpret_cast<TextureCubeApp*>(glfwGetWindowUserPointer(window));
	app->mZoomLevel += int(yOffset);
	app->mZoomLevel = glm::clamp(app->mZoomLevel, -5, 5);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	auto app = reinterpret_cast<TextureCubeApp*>(glfwGetWindowUserPointer(window));
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		app->mMouseDrag = true;
		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		app->mTrackball.startDrag(glm::ivec2(int(mouseX), int(mouseY)));
	} else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		app->mTrackball.endDrag();
		app->mMouseDrag = false;
	}
}

void TextureCubeApp::initVulkan() {
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
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createTextureImages();
	createTextureImageViews();
	createTextureSamplers();
	loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}

void TextureCubeApp::mainLoop() {
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(mDevice);
}

void TextureCubeApp::cleanup() {
	cleanupSwapChain();

	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
	}

	vkDestroySampler(mDevice, mSpecularTextureSampler, nullptr);
	vkDestroyImageView(mDevice, mSpecularTextureImageView, nullptr);
	vkDestroyImage(mDevice, mSpecularTextureImage, nullptr);
	vkFreeMemory(mDevice, mSpecularTextureImageMemory, nullptr);

	vkDestroySampler(mDevice, mDiffuseTextureSampler, nullptr);
	vkDestroyImageView(mDevice, mDiffuseTextureImageView, nullptr);
	vkDestroyImage(mDevice, mDiffuseTextureImage, nullptr);
	vkFreeMemory(mDevice, mDiffuseTextureImageMemory, nullptr);

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

void TextureCubeApp::createInstance() {
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
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Finally, we are ready to create the instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);
	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}
