#include <cstring>

#include <iostream>
#include <vector>

#include "TriangleApp.h"

std::vector<const char*> TriangleApp::getRequiredExtensions() {
	// Query which extensions requiere this windows manager: GLFW
	// The answer is platform specific!!
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(/* begin = */glfwExtensions,
		/* end = */glfwExtensions + glfwExtensionCount);

	// If we want validation layer we add this extension to the requied list
	if (mEnableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void TriangleApp::validateExtensions(const std::vector<const char*>& requiredExt) {
	// Add logic to query the number of extension present in this driver
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::cout << "Number of supported extensions: " << extensionCount << std::endl;
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
	std::cout << "Available extensions:" << std::endl;
	for (const auto& availExt : availableExtensions) {
		std::cout << '\t' << availExt.extensionName << std::endl;
	}
	// List the number of required extensions by this window manager
	std::cout << "Number of required extensions by GLFW: " << requiredExt.size() << std::endl;
	std::cout << "Required extensions by GLFW:" << std::endl;
	for (const auto& extension : requiredExt) {
		std::cout << '\t' << extension << std::endl;
	}
	// Check that we have the needed ones
	std::cout << "Missing extensions:" << std::endl;
	for (const auto& reqExt : requiredExt) {
		bool present{ false };
		// Looking for the extension...
		for (const auto& availExt : availableExtensions) {
			// Found it!
			if (strcmp(reqExt, availExt.extensionName) == 0) {
				present = true;
				break;
			}
		}
		// If not found...
		if (!present) {
			std::cout << '\t' << reqExt << " not present!" << std::endl;
		}
	}
}
/*
* Check if the validation layers are going to be supported supported in this instance 
*/
bool TriangleApp::checkValidationLayerSupport() {
	uint32_t layerCount;
	// To query the available layers first we query their number
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	// Allocate space to store the query results
	std::vector<VkLayerProperties> availableLayers(layerCount);
	// Query all availavle layers
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	// Verify that the validation layers are among them
	for (const char* layerName : mValidationLayers) {
		bool layerFound = false;
		// Loop trough all the available layers to see if this valiation layer is there
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) { // We found it!
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}
