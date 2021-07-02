#include "TriangleApp.h"

void TriangleApp::createFramebuffers() {
	// Make sure that the container can hold all the required FB (one per image in the swapchain)
	mSwapChainFramebuffers.resize(mSwapChainImageViews.size());
	// Create one frambuffer per imageView in the swapchain
	for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
		VkImageView attachments[] = {
			mSwapChainImageViews[i] // View need to be provided as an array
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass; // Our render pass needs to be compatible
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = mSwapChainExtent.width;
		framebufferInfo.height = mSwapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
/*
* We have to create a command pool before we can create command buffers. 
* Command pools manage the memory that is used to store the buffers and 
* command buffers are allocated from them
*/
void TriangleApp::createCommandPool() {
	// Since each command buffer can only submit one queue family, the pool needs to refer to them
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(mPhysicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // We are drawing, we will use the grafics queue
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

}

void TriangleApp::createCommandBuffers() {
	// We need to create one command buffer per each image in the swapchain
	mCommandBuffers.resize(mSwapChainFramebuffers.size());
	// We allocate memmory (from the command pool) for the command buffers
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

	if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < mCommandBuffers.size(); i++) {
		// We need to mark the beggining of the command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Not Optional anymore !
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		// We also need to let him know to which render pass and which framebuffer
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapChainFramebuffers[i];
		// We will render the whole buffer
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = mSwapChainExtent;
		// Clear the FB to black with 100 opacity
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		/* Recording the commands */
		vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		// Bind the desired pipeline 
		vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
		// Actual render command
		vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);
		// End the render pass
		vkCmdEndRenderPass(mCommandBuffers[i]);

		if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void TriangleApp::createSyncObjects() {

	mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	// This ones will be used as references for mapping into the InFlightFences array (That is why they are not created)
	mImagesInFlight.resize(mSwapChainImages.size(), VK_NULL_HANDLE);
	// For the semaphores
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	// For the fences
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) 
				!= VK_SUCCESS ||
			vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i])
				!= VK_SUCCESS ||
			vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i])
				!= VK_SUCCESS) {

			throw std::runtime_error("failed to create sync objects for a frame!");
		}
	}
	
}

void TriangleApp::drawFrame() {
	vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);
	
	uint32_t imageIndex;
	// Query for the index of the next available image in the swapchain
	// We also pass down a sync object to signal
	VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX,
		mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
	// Check that the swapchain still matches this surface
	// Most driver will trhow this when the window is resized, but it is not garanted
	// Look bellow (after presnetation), to know how to handle in case they don't signal
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	// Check if a previous frame is using this image (i.e. there is a fence to wait on)
	if (mImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(mDevice, 1, &mImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	mImagesInFlight[imageIndex] = mInFlightFences[mCurrentFrame];

	// Prepare to submit commands to the queue
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	// Set of conditions (we only have one) to wait before executing
	VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
	// At which stage to wait
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	// Indices makes a correspondence between the two arrays
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	// select the buffer to submit (using the image index)
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
	// Set of conditions (again, only one) to signal once we finish
	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	// Reset the fence just before using it again in the graphics queue
	vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);
	
	// Submit to the queue
	if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	// Prepare to present the frame
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	// Semaphores to wait before presentation can happen (in this case the one was signal after we finish render)
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	// To which swapchains we want to present (Usually, just one)
	VkSwapchainKHR swapChains[] = { mSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	// And which index in the swapchain to present
	presentInfo.pImageIndices = &imageIndex;
	// An optional array of return values, to check for errors in each swapchain. Since we just have one
	// we will not use it. We will use the return value to see if there were errors
	presentInfo.pResults = nullptr; // Optional
	// Present the frame
	result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
	// If we present the frame but does not match the surface, we need to recreate the swapchain
	// We need to do it after presenting the surface or the semaphores could end out of sync
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
		mFramebufferResized = false; // We are finally sure that we handle the resize
		recreateSwapChain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	// Advance to the next frame
	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}