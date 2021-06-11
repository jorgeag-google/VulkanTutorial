#include <iostream>
#include <set>
#include <stdexcept>

#include "TextureMapApp.h"
#include "Device.h"

void TextureMapApp::pickPhysicalDevice() {
	// Query the number of vulkan supported GPU's
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	// std::cout << "Number of Vulkan capable GPUs found: " << deviceCount << std::endl;
	// List all of the available devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	// Find a suitable GPU of all the available ones
	// Beside of being Vulkan capable needs to be abe to do the graphics we need
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			mPhysicalDevice = device;
			break;
		}
	}
	if (mPhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

SwapChainSupportDetails TextureMapApp::querySwapChainSupport(VkPhysicalDevice device) {
	// Fill the dteails struct by using the device and the surface form this objet
	// Before this call we need to already have a valid surface handle in this object
	SwapChainSupportDetails details;
	// First, capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);
	// Second the formats (since this is a list) we need a vector to hold it
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
	}
	// Third the present modes 
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool TextureMapApp::isDeviceSuitable(VkPhysicalDevice device) {
	// Query for both properties and features
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	/* Sample of a query for specific properties */
	/*
	// If this device is not a GPU, OR does not have the hability of using geometri shaders
	if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
		!deviceFeatures.geometryShader) {
		// Then it is not suitable
		return false;
	}
	*/

	// Check that this device has the correct queues that we will use
	QueueFamilyIndices indices = findQueueFamilies(device);
	// Check that this device supports all the extension we will need
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	// If the extension is supported the see if the swapchain is compatible with the surface
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && 
		swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void TextureMapApp::createLogicalDevice() {
	// In order to create the device we need two structures

	// First, the queues structures
	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	// Set is to remove the duplicity, since it could be that
	// the same queue support more than one operation, in this case,
	// several indices are the same
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		// Add this queueInfo to the list
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Second, the device fetaures
	// For now, this can be left empthy ()
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// Now, that we have those two structs, we can create our logical device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	// Register the previosly created structs
	// it could be more than one queCreateInfo (if a queue does 
	// not support all our operations). But, most likely there will be just one
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	// This, is not strictlly needed in modern drivers,
	// (they just ignore it)
	// it put it here for backward compatibility
	if (mEnableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	// Enable the required extensions on this device
	createInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

	// Finally, try to create the logical device
	if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	// We can adquire the queues handles too, since we just adquiere the device
	vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
}

bool TextureMapApp::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	// Query the avalable extensions on this device
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
	// Place the required extensions in this set
	std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());
	// Check that all the requied extensions are available
	// by eliminating the available ones from the set of required
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	// See if we eliminate all the required from the set
	return requiredExtensions.empty();
}

QueueFamilyIndices TextureMapApp::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		// Check if this queue can render to our surface
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		// If we already have all our queues we can early exit
		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}
