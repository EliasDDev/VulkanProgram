#include <iostream>
#include <fstream>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <vulkan/vulkan.hpp>

std::string readFile(std::string filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	assert(file.is_open());

	size_t fileSize = file.tellg();
	std::string buffer(fileSize, ' ');

	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);

	file.close();

	return buffer;
}

int main()
{
	// Variables

	const char* validationLayers = "VK_LAYER_KHRONOS_validation";
	uint32_t queueFamilyIndex = 0;
	const char* deviceExtensions = "VK_KHR_swapchain";

	// Window

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(960, 540, "Vulkan", nullptr, nullptr);

	// Instance

	vk::ApplicationInfo applicationInfo = {};
	applicationInfo.pApplicationName = nullptr;
	applicationInfo.applicationVersion = 1;
	applicationInfo.pEngineName = nullptr;
	applicationInfo.engineVersion = 1;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = nullptr;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vk::InstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = 1;
	instanceCreateInfo.ppEnabledLayerNames = &validationLayers;
	instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

	vk::Instance instance = vk::createInstance(instanceCreateInfo);

	// Surface

	VkSurfaceKHR surface;
	glfwCreateWindowSurface(instance, window, nullptr, &surface);

	// Physical device and queue families

	vk::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices().front();

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			if (!physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, surface))
			{
				continue;
			}

			queueFamilyIndex = i;
			break;
		}
	}

	// Logical device and queues

	float queuePriorities = 1.0f;

	vk::DeviceQueueCreateInfo deviceQueueCreateInfo = {};
	deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = &queuePriorities;

	vk::DeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 1;
	deviceCreateInfo.ppEnabledLayerNames = &validationLayers;
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = &deviceExtensions;

	vk::Device device = physicalDevice.createDevice(deviceCreateInfo);

	vk::Queue graphicsQueue = device.getQueue(queueFamilyIndex, 0);
	vk::Queue presentQueue = device.getQueue(queueFamilyIndex, 0);

	// Swapchain

	vk::SurfaceFormatKHR surfaceFormatKHR = {};
	std::vector<vk::SurfaceFormatKHR> avaliableSurfaceFormatsKHR = physicalDevice.getSurfaceFormatsKHR(surface);
	for (const auto& surfaceFormat : avaliableSurfaceFormatsKHR)
	{
		if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			surfaceFormatKHR = surfaceFormat;
			break;
		}

		surfaceFormatKHR = avaliableSurfaceFormatsKHR.front();
	}

	vk::PresentModeKHR presentModeKHR = {};
	std::vector<vk::PresentModeKHR> avaliablePresentModesKHR = physicalDevice.getSurfacePresentModesKHR(surface);
	for (const auto& presentMode : avaliablePresentModesKHR) {
		if (presentMode == vk::PresentModeKHR::eMailbox) {
			presentModeKHR = presentMode;
			break;
		}

		presentModeKHR = vk::PresentModeKHR::eFifo;
	}

	vk::SurfaceCapabilitiesKHR surfaceCapabilitiesKHR = physicalDevice.getSurfaceCapabilitiesKHR(surface);

	vk::SwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
	swapchainCreateInfoKHR.surface = surface;
	swapchainCreateInfoKHR.minImageCount = surfaceCapabilitiesKHR.minImageCount;
	swapchainCreateInfoKHR.imageFormat = surfaceFormatKHR.format;
	swapchainCreateInfoKHR.imageColorSpace = surfaceFormatKHR.colorSpace;
	swapchainCreateInfoKHR.imageExtent = surfaceCapabilitiesKHR.currentExtent;
	swapchainCreateInfoKHR.imageArrayLayers = 1;
	swapchainCreateInfoKHR.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapchainCreateInfoKHR.imageSharingMode = vk::SharingMode::eExclusive;
	swapchainCreateInfoKHR.preTransform = surfaceCapabilitiesKHR.currentTransform;
	swapchainCreateInfoKHR.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchainCreateInfoKHR.presentMode = presentModeKHR;
	swapchainCreateInfoKHR.clipped = true;
	swapchainCreateInfoKHR.oldSwapchain = nullptr;

	vk::SwapchainKHR swapchainKHR = device.createSwapchainKHR(swapchainCreateInfoKHR);

	std::vector<vk::Image> swapchainImages = device.getSwapchainImagesKHR(swapchainKHR);

	// Image views

	std::vector<vk::ImageView> swapchainImageViews;
	swapchainImageViews.resize(swapchainImages.size());
	for (size_t i = 0; i < swapchainImageViews.size(); i++)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = surfaceFormatKHR.format;
		imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		swapchainImageViews[i] = device.createImageView(imageViewCreateInfo);
	}

	// Render passes

	vk::AttachmentDescription attachmentDescription = {};
	attachmentDescription.format = surfaceFormatKHR.format;
	attachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
	attachmentDescription.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference attachmentReference = {};
	attachmentReference.attachment = 0;
	attachmentReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpassDescription = {};
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &attachmentReference;

	vk::SubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; // TODO: see if there is a vulkan.hpp way of doing this
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	subpassDependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	vk::RenderPass renderPass = device.createRenderPass(renderPassCreateInfo);

	// Shader modules

	auto vertexShaderCode = readFile("Resources/vert.spv");
	auto fragmentShaderCode = readFile("Resources/frag.spv");

	vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo = {};
	vertexShaderModuleCreateInfo.codeSize = vertexShaderCode.size();
	vertexShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderCode.data());

	vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {};
	fragmentShaderModuleCreateInfo.codeSize = fragmentShaderCode.size();
	fragmentShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderCode.data());

	vk::ShaderModule vertexShaderModule = device.createShaderModule(vertexShaderModuleCreateInfo);
	vk::ShaderModule fragmentShaderModule = device.createShaderModule(fragmentShaderModuleCreateInfo);

	vk::PipelineShaderStageCreateInfo vertexPipelineShaderStageCreateInfo = {};
	vertexPipelineShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertexPipelineShaderStageCreateInfo.module = vertexShaderModule;
	vertexPipelineShaderStageCreateInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo fragmentPipelineShaderStageCreateInfo = {};
	fragmentPipelineShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragmentPipelineShaderStageCreateInfo.module = fragmentShaderModule;
	fragmentPipelineShaderStageCreateInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[] = { vertexPipelineShaderStageCreateInfo , fragmentPipelineShaderStageCreateInfo };

	// Fixed functions

	vk::PipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.vertexBindingDescriptionCount = 0;
	vertexInputState.pVertexBindingDescriptions = nullptr;
	vertexInputState.vertexAttributeDescriptionCount = 0;
	vertexInputState.pVertexAttributeDescriptions = nullptr;

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssemblyState.primitiveRestartEnable = false;

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = surfaceCapabilitiesKHR.currentExtent.width;
	viewport.height = surfaceCapabilitiesKHR.currentExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor = {};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = surfaceCapabilitiesKHR.currentExtent;

	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.depthClampEnable = false;
	rasterizationState.rasterizerDiscardEnable = false;
	rasterizationState.polygonMode = vk::PolygonMode::eFill;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
	rasterizationState.frontFace = vk::FrontFace::eClockwise;
	rasterizationState.depthBiasEnable = false;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = false;

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};

	vk::PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	// Graphics pipeline

	vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	graphicsPipelineCreateInfo.pViewportState = &viewportState;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
	graphicsPipelineCreateInfo.pMultisampleState = nullptr;
	graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;

	vk::ResultValue<vk::Pipeline> result = device.createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
	assert(result.result == vk::Result::eSuccess);
	vk::Pipeline pipeline = result.value;

	// Framebuffers

	std::vector<vk::Framebuffer> swapchainFramebuffers(swapchainImageViews.size());

	for (size_t i = 0; i < swapchainFramebuffers.size(); i++)
	{
		vk::FramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &swapchainImageViews[i];
		framebufferCreateInfo.width = surfaceCapabilitiesKHR.currentExtent.width;
		framebufferCreateInfo.height = surfaceCapabilitiesKHR.currentExtent.height;
		framebufferCreateInfo.layers = 1;

		swapchainFramebuffers[i] = device.createFramebuffer(framebufferCreateInfo);
	}

	device.destroyShaderModule(fragmentShaderModule);
	device.destroyShaderModule(vertexShaderModule);

	// Command buffers

	vk::CommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

	vk::CommandPool commandPool = device.createCommandPool(commandPoolCreateInfo);

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo).front();

	// Rendering and presentation

	vk::SemaphoreCreateInfo semaphoreCreateInfo = {};

	vk::FenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	vk::Semaphore imageAvailableSemaphore = device.createSemaphore(semaphoreCreateInfo);
	vk::Semaphore renderFinishedSemaphore = device.createSemaphore(semaphoreCreateInfo);
	vk::Fence inFlightFence = device.createFence(fenceCreateInfo);

	std::cout << "Vulkan: " << physicalDevice.getProperties().deviceName << std::endl;

	uint32_t imageIndex = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		device.waitForFences(inFlightFence, true, UINT32_MAX); // TODO: this may not work, see vulkan-tutorial

		device.resetFences(inFlightFence); // TODO: same here as above

		device.acquireNextImageKHR(swapchainKHR, UINT64_MAX, imageAvailableSemaphore, nullptr, &imageIndex);

		commandBuffer.reset();

		vk::CommandBufferBeginInfo commandBufferBeginInfo = {};

		commandBuffer.begin(commandBufferBeginInfo);

		vk::RenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
		renderPassBeginInfo.renderArea.offset = vk::Offset2D(0, 0);
		renderPassBeginInfo.renderArea.extent = surfaceCapabilitiesKHR.currentExtent;
		renderPassBeginInfo.clearValueCount = 1;

		vk::ClearValue clearValue = {};
		renderPassBeginInfo.pClearValues = &clearValue;

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

		commandBuffer.draw(3, 1, 0, 0);

		commandBuffer.endRenderPass();

		commandBuffer.end();

		vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		vk::SubmitInfo submitInfo = {};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = &flags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
		graphicsQueue.submit(submitInfo, inFlightFence);

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchainKHR;
		presentInfo.pImageIndices = &imageIndex;

		presentQueue.presentKHR(presentInfo);
	}

	device.waitIdle();

	device.destroyFence(inFlightFence);
	device.destroySemaphore(renderFinishedSemaphore);
	device.destroySemaphore(imageAvailableSemaphore);

	device.destroyCommandPool(commandPool);

	for (const auto& framebuffer : swapchainFramebuffers)
	{
		device.destroyFramebuffer(framebuffer);
	}

	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipelineLayout);
	device.destroyRenderPass(renderPass);

	for (const auto& imageView : swapchainImageViews)
	{
		device.destroyImageView(imageView);
	}

	device.destroySwapchainKHR(swapchainKHR);
	instance.destroySurfaceKHR(surface);
	device.destroy();
	instance.destroy();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
