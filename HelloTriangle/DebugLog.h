#pragma once

#include <string>

// For vulkan related datatypes
#include "TriangleApp.h"
// Format an error message into an string with all the info we desire to print
std::string debugMgsg2str(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

// This is the debug callback for an event in the validation layers
// it is a static function so it does not have access to the app class
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	// User defined filter
	// Only print it if the message is a warning or an error
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cerr << "Validation layer" << std::endl;
		std::cerr << debugMgsg2str(messageSeverity, messageType, pCallbackData) << std::endl;
	}

	return VK_FALSE;
}
// Since the function we need to use comes from an extension. It is not automatically loaded.
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
// Since the function we need to use comes from an extension. It is not automatically loaded.
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);