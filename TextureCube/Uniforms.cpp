#include <array>
#include <chrono>

#include "Uniforms.h"
#include "TextureCubeApp.h"

void TextureCubeApp::createDescriptorSetLayout() {

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	// In which stage of the pipeline
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding specularSamplerLayoutBinding{};
	specularSamplerLayoutBinding.binding = 1;
	specularSamplerLayoutBinding.descriptorCount = 1;
	specularSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularSamplerLayoutBinding.pImmutableSamplers = nullptr;
	specularSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding diffuseSamplerLayoutBinding{};
	diffuseSamplerLayoutBinding.binding = 2;
	diffuseSamplerLayoutBinding.descriptorCount = 1;
	diffuseSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseSamplerLayoutBinding.pImmutableSamplers = nullptr;
	diffuseSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, 
		specularSamplerLayoutBinding, diffuseSamplerLayoutBinding };
	// Prepare to create the descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void TextureCubeApp::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	mUniformBuffers.resize(mSwapChainImages.size());
	mUniformBuffersMemory.resize(mSwapChainImages.size());

	for (size_t i = 0; i < mSwapChainImages.size(); i++) {
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				mUniformBuffers[i], mUniformBuffersMemory[i]);
	}
}

void TextureCubeApp::updateUniformBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	// get the time between franmes
	float time = 
		std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	// Calculate the uniform's values for this frame
	UniformBufferObject ubo{};
	// Model
	ubo.model = glm::rotate(/*startFrom=*/glm::mat4(1.0f),
		/*angle=*/mRotate ? time * glm::radians(90.0f) : 0.0f,
		/*axis=*/glm::vec3(0.0f, 1.0f, 0.0f));
	// View
	ubo.view = glm::lookAt(
		/*eyePos=*/glm::vec3(0.0f, 0.0f, 3.0f),
		/*center=*/glm::vec3(0.0f, 0.0f, 0.0f),
		/*up=*/glm::vec3(0.0f, 1.0f, 0.0f)) * mTrackball.getRotation();
	// Projection
	const float TAU = 6.28318f; //Math constant equal two PI
	float angle = TAU / 8.0f + mZoomLevel * (TAU / 50.0f);
	ubo.proj = glm::perspective(
		/*fov-Y=*/angle,
		/*aspect=*/mSwapChainExtent.width / (float)mSwapChainExtent.height,
		/*near=*/1.0f,
		/*far=*/5.0f);
	
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

void TextureCubeApp::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 3> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(mSwapChainImages.size());

	if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void TextureCubeApp::createDescriptorSets() {
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

		VkDescriptorImageInfo imageInfoSpecular{};
		imageInfoSpecular.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfoSpecular.imageView = mSpecularTextureImageView;
		imageInfoSpecular.sampler = mSpecularTextureSampler;

		VkDescriptorImageInfo imageInfoDiffuse{};
		imageInfoDiffuse.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfoDiffuse.imageView = mDiffuseTextureImageView;
		imageInfoDiffuse.sampler = mDiffuseTextureSampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = mDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0; // As described in the shader
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = mDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfoSpecular;
		
		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = mDescriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imageInfoDiffuse;
		
		vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(descriptorWrites.size()), 
				descriptorWrites.data(), 0, nullptr);
	}
}