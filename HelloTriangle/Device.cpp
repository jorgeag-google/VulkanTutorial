#include <iostream>
#include <set>
#include <stdexcept>

#include "TriangleApp.h"
#include "Device.h"

void TriangleApp::pickPhysicalDevice() {
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

SwapChainSupportDetails TriangleApp::querySwapChainSupport(VkPhysicalDevice device) {
	// Fill the details struct by using the device and the surface from this objet
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

bool TriangleApp::isDeviceSuitable(VkPhysicalDevice device) {
	// Query for both properties and features
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

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
	// beetween them is the swapchain extension
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		// If the extension is supported, then see if the swapchain is compatible with the surface
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void TriangleApp::createLogicalDevice() {
	// In order to create the device we need two structures
	
	// First, the queues structures
	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	// Set is to remove the duplicity, since it could be that
	// the same queue supports more than one operation. If that is the case,
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

	// Second, the device features
	// For now, this can be left empthy ()
	VkPhysicalDeviceFeatures deviceFeatures{};

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
	} else {
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
	// It could be that both handles are the same (See above)
	vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
}

bool TriangleApp::checkDeviceExtensionSupport(VkPhysicalDevice device) {
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

QueueFamilyIndices TriangleApp::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;
	// Queary how many queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	// get the list of families
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0; // We register the queues by their index in the list
	// Loop trough all the queues families to fullfill all our needs
	// Note that one queue could support of all our required features
	for (const auto& queueFamily : queueFamilies) {
		// If this queue support graphics
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		// Check if this queue can render to our surface
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}
		// If we already have all our queue indices we can exit
		if (indices.isComplete()) {
			break;
		}
		// increment the queue index
		i++;
	}

	return indices;
}
