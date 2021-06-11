#include "Vertex.h"
#include "LoadModelApp.h"

void LoadModelApp::createVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(mVertices[0]) * mVertices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, stagingBufferMemory);
	// Fill the vertex buffer with the data
	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mVertices.data(), (size_t)bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);
	// Copy the stagging buffer (which is on host shared mem) into the Vertex buffer 
	// (which is on device vid mem)
	copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);
	// Destory the stagging buffer (we do not need it anymore)
	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void LoadModelApp::createIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(mIndices[0]) * mIndices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);
	// Fill the vertex buffer with the data
	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mIndices.data(), (size_t)bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);
	// Copy the stagging buffer (which is on host shared mem) into the Vertex buffer 
	// (which is on device vid mem)
	copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);
	// Destory the stagging buffer (we do not need it anymore)
	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void LoadModelApp::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	// Prepare the buffer creation
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// Create buffer
	if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}
	// Query the mem requieriments
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);
	// Prepare to allocate the mem
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
	// Allocate memmory
	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}
	// Bind the buffer and his memmory
	vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
}

uint32_t LoadModelApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (  typeFilter & (1 << i) && 
			  (memProperties.memoryTypes[i].propertyFlags & properties) == properties
		   ) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void LoadModelApp::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	// Copies are command submitted to queues, so we need to create a command buffer
	// to copy buffers
	// Create command buffer
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	// Only one command the copy operation
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	// Finish the command buffer
	endSingleTimeCommands(commandBuffer);
}

void LoadModelApp::createDepthResources() {
	// Query the best depth format available
	VkFormat depthFormat = findDepthFormat();
	// Create image
	createImage(mSwapChainExtent.width, mSwapChainExtent.height, depthFormat, 
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage, mDepthImageMemory);
	// Now view
	mDepthImageView = createImageView(mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	// Transition to an depth/stencil buffer (optional in our case)
	transitionImageLayout(mDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat LoadModelApp::findSupportedFormat(const std::vector<VkFormat>& candidates,
	VkImageTiling tiling, VkFormatFeatureFlags features) {

	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && 
			(props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && 
			(props.optimalTilingFeatures & features) == features) {
			return format;
		}

	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat LoadModelApp::findDepthFormat() {
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool LoadModelApp::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
