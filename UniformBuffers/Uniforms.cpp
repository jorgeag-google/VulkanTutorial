#include <chrono>

#include "Uniforms.h"
#include "UniformBufferApp.h"

void UniformBufferApp::createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	// In which stage of the pipeline
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
	// Prepare to create the descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void UniformBufferApp::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	mUniformBuffers.resize(mSwapChainImages.size());
	mUniformBuffersMemory.resize(mSwapChainImages.size());

	for (size_t i = 0; i < mSwapChainImages.size(); i++) {
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				mUniformBuffers[i], mUniformBuffersMemory[i]);
	}
}

void UniformBufferApp::updateUniformBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	// get the timw between franmes
	float time = 
		std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	// Calculate the uniform's values for this frame
	UniformBufferObject ubo{};
	// Model
	ubo.model = glm::rotate(/*startFrom=*/glm::mat4(1.0f), 
		/*angle=*/time * glm::radians(90.0f),
		/*axis=*/glm::vec3(0.0f, 0.0f, 1.0f));
	// View
	ubo.view = glm::lookAt(
		/*eyePos=*/glm::vec3(2.0f, 2.0f, 2.0f),
		/*center=*/glm::vec3(0.0f, 0.0f, 0.0f),
		/*up=*/glm::vec3(0.0f, 0.0f, 1.0f));
	// Projection
	ubo.proj = glm::perspective(
		/*fov-Y=*/glm::radians(45.0f),
		/*aspect=*/mSwapChainExtent.width / (float)mSwapChainExtent.height,
		/*near=*/0.1f,
		/*far=*/10.0f);
	
	/* 
	   GLM was originally designed for OpenGL, where the Y coordinate of the clip 
	   coordinates is inverted. The easiest way to compensate for that is to flip 
	   the sign on the scaling factor of the Y axis in the projection matrix.
	   
	   If you don't do this, then the image will be rendered upside down.
	*/
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(mDevice, mUniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(mDevice, mUniformBuffersMemory[currentImage]);

}

void UniformBufferApp::createDescriptorPool() {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());
	
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(mSwapChainImages.size());

	if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void UniformBufferApp::createDescriptorSets() {
	// We need one descriptor set per image in the swapcahin
	std::vector<VkDescriptorSetLayout> layouts(mSwapChainImages.size(), mDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	mDescriptorSets.resize(mSwapChainImages.size());

	if (vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < mSwapChainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mDescriptorSets[i];
		descriptorWrite.dstBinding = 0; // As specified in the shader
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
	}


}