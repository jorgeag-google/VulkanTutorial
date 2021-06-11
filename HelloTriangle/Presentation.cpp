#include <cstdint> // Necessary for UINT32_MAX
#include <algorithm> // Necessary for std::min/std::max

#include "TriangleApp.h"

void TriangleApp::createSurface() {
	if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void TriangleApp::recreateSwapChain() {
	
	int width = 0, height = 0;
	glfwGetFramebufferSize(mWindow, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(mWindow, &width, &height);
		glfwWaitEvents();
	}
	
	vkDeviceWaitIdle(mDevice);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandBuffers();
}

void TriangleApp::cleanupSwapChain() {
	for (size_t i = 0; i < mSwapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(mDevice, mSwapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(mDevice, mCommandPool, 
		static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

	for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
		vkDestroyImageView(mDevice, mSwapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
}

void TriangleApp::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice);
	// We chose the best format, present mode and extent form all the available
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
	// Now we specify how many image we will require
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	// Make sur we do not exced the maximum
	if (// zero is a special value that means no maxmimum
			swapChainSupport.capabilities.maxImageCount > 0 && 
			imageCount > swapChainSupport.capabilities.maxImageCount
		) {	
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	// Prepare the info for creating the swapchain
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;
	// These details corresponds to the images and are the one we just calculated
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Keep a copy of the format and the extend since we will need it
	mSwapChainImageFormat = surfaceFormat.format;
	mSwapChainExtent = extent;

	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// If I do not provide this two, the validation layer presents this
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());
}

void TriangleApp::createImageViews() {
	// Rezise array to hold the number of required views
	// (one per image)
	mSwapChainImageViews.resize(mSwapChainImages.size());
	// Create the images
	for (size_t i = 0; i < mSwapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = mSwapChainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = mSwapChainImageFormat;
		// No swizzle
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		// How to use this images (this config is for simple render targets)
		// No multiple layers, no mipamap levels
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

VkSurfaceFormatKHR TriangleApp::chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// Loop trough all the available formats
	for (const auto& availableFormat : availableFormats) {
		if (// Color channels and types (this a 32 bytes pixel format)
			availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
			// check the SRGB space ai avalable
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {

			// Stop at the first one that fulfils our needs
			return availableFormat;
		}
	}
	// If none just settle for the first one
	return availableFormats[0];
}

VkPresentModeKHR TriangleApp::chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR>& availablePresentModes) {

	// Look for the triple buffering format
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	// If not settle with this one (this is the only one garanteed to always exists)
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D TriangleApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	// Special values to say that Vulkan will match the window resolution and he will
	// not let us choose.
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		// Get windoe width and height
		int width, height;
		// We need to query them, since the screen coordinats and the pixel size might differ
		// for example in retina display. Therefore we need to ask for the resolution in pixels
		glfwGetFramebufferSize(mWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		// Clamp the desired values between the min and max available
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}