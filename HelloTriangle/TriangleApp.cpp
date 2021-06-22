#include <stdexcept>

#include "TriangleApp.h"
#include "DebugLog.h"


/*
* This is the entry point of the execution
* The only public method
*/
void TriangleApp::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

/*
* This handles window creation and the register of callback functions
*/
void TriangleApp::initWindow() {
	// This need to be called before doing anithing else
	glfwInit();
	// Since GLFW, was designed for OpenGL, we need to explicitlly tell him not to create
	// an OGL context. Thi might change for future versions of GLFW
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// Create the window an keep the handle, since we will need it to release resources at the end
	mWindow = glfwCreateWindow(mWidth, mHeight, "Hello Triangle in Vulkan", nullptr, nullptr);
	// Register this pointer with GLFW (so we can have public access to the class in static callbacks)
	glfwSetWindowUserPointer(mWindow, this);
	// Register the callback function for window and interaction events
	glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback); // Callback for window resize
}

/*
* An static non-member callback function. It's called when window gets resized
*/
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	// Recover a reference to the app object (we only have public access)
	auto app = reinterpret_cast<TriangleApp*>(glfwGetWindowUserPointer(window));
	app->mFramebufferResized = true; // The window has been resized and we have not handle it yet
}

/*
* This is the most important funtion for the porpouse of the tutorial. 
* It handles all the vulkan initialization logic.
*/
void TriangleApp::initVulkan() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
}

/*
* Mainloop: after intialization, the app enters this state and it will
* keep handling events and drawing frames until an exit signal is detected
*/
void TriangleApp::mainLoop() {
	// While the window has not been signal to close
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents(); // Handle any event register in the callbacks
		drawFrame(); //draw another frame
	}

	vkDeviceWaitIdle(mDevice);
}

/*
* It is called when the app is about to exit. It must free any resource
* used by the app. It could be a Vulkan resource, a GLFW resource or an
* application specific resource
*/
void TriangleApp::cleanup() {
	cleanupSwapChain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	vkDestroyDevice(mDevice, nullptr);

	if (mEnableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
	// Free the GLFW resources
	glfwDestroyWindow(mWindow); // The window
	glfwTerminate(); // the GLFW framework
}

void TriangleApp::createInstance() {
	if (mEnableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Optional app info struct help us configure the instance
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	// This struct holds the instance creation option
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo; // Link to the app info (optional)
	// query the required extensions
	auto extensions = getRequiredExtensions();
	// Validate that we have at least the required extensions
	// validateExtensions(extensions);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	// If we want the validation layers, we request them here
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (mEnableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();
		// Here we setup this debug to have a mechanism to debug the instance and device creation
		// since, this will happen before setting up our more general debug logger
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
