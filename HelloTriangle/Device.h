#pragma once

#include <cstdint>

#include <vector>
#include <optional>

// To get the Vulkan datatypes
#include "TriangleApp.h"

// Handles the indices af the queues of each family needed
struct QueueFamilyIndices {
	// Note that it could be that same queue supports both families
	// in that case both indices will be the same
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	inline bool isComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

// To encapsulate all the details needed to select a swapchain in one place
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
