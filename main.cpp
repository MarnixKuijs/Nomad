#include "GPU/GPUContext.h"
#include "GPU/CommandPool.h"
#include "GPU/Shader.h"
#include "GPU/Semaphore.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderPass.h"
#include "Utils/VulkanUtils.h"
#include "Graphics/Vertex.h"

#define WIN32_LEAN_AND_MEAN
#include <vulkan/vulkan.h>
#undef WIN32_LEAN_AND_MEAN
#include <GLFW/glfw3.h>

#include <array>
#include <string_view>
#include <assert.h>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#if defined(_DEBUG)
constexpr static std::array layers
{
	"VK_LAYER_LUNARG_standard_validation"
};
#else
constexpr static std::array<const char*, 0> layers{};
#endif

static std::vector<const char*> desiredInstanceExtensions{};

static VkApplicationInfo appInfo
{
	VK_STRUCTURE_TYPE_APPLICATION_INFO,
	nullptr,
	"Nomad",
	1u,
	"LunarG SDK",
	1u,
	VK_API_VERSION_1_1
};

constexpr static uint64_t desiredFeaturesBitMask{ 1 | 1 << 1 | 1 << 2 | 1 << 3 };
constexpr static VkQueueFlags queueFlags{ VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT };
static std::vector<const char*> desiredDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

void createBuffer(const cof::GPUContext& gpuContext,VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(gpuContext.LogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(gpuContext.LogicalDevice(), buffer, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(gpuContext.PhysicalDevice(), &memProperties);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = [&memProperties, &memRequirements](VkMemoryPropertyFlags properties)
	{
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if ((memRequirements.memoryTypeBits & (1u << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		assert(false);
		return std::numeric_limits<uint32_t>::max();
	}(properties);

	if (vkAllocateMemory(gpuContext.LogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(gpuContext.LogicalDevice(), buffer, bufferMemory, 0);
}

int main()
{

#ifdef VK_USE_PLATFORM_WIN32_KHR
	puts("windows");
#elif VK_USE_PLATFORM_XLIB_KHR
	puts("linux");
#endif

	[[maybe_unused]] int glfwInitStatus = glfwInit();
	assert(glfwInitStatus);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(640, 480, "Nomad", nullptr, nullptr);

	[[maybe_unused]] VkResult errorCode{ VK_RESULT_MAX_ENUM };

	uint32_t instanceExtensionsCount{ 0 };
	errorCode = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);
	assert(errorCode == VK_SUCCESS || instanceExtensionsCount != 0);

	std::vector<VkExtensionProperties> availableInstanceExtensions{ instanceExtensionsCount };
	errorCode = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, availableInstanceExtensions.data());
	assert(errorCode == VK_SUCCESS);

	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	desiredInstanceExtensions.reserve(desiredInstanceExtensions.size() + static_cast<size_t>(glfwExtensionCount));
	desiredInstanceExtensions.insert(desiredInstanceExtensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

	for (auto& desiredInstanceextension : desiredInstanceExtensions)
	{
		assert(IsExtensionSupported(availableInstanceExtensions, desiredInstanceextension));
	}

	static VkInstanceCreateInfo instInfo
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0u,
		&appInfo,
		static_cast<uint32_t>(layers.size()),
		layers.data(),
		static_cast<uint32_t>(desiredInstanceExtensions.size()),
		desiredInstanceExtensions.data()
 	};

	static VkInstance instance;
 	errorCode = vkCreateInstance(&instInfo, nullptr, &instance);
 	assert(errorCode == VK_SUCCESS);

	static VkSurfaceKHR surface;
	errorCode = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	assert(errorCode == VK_SUCCESS);

	cof::GPUContext gpuContext{ instance, desiredFeaturesBitMask, queueFlags, desiredDeviceExtensions };

	const auto& queueFamilyIndices = gpuContext.QueueFamilyIndices();
	const auto physicalDevice = gpuContext.PhysicalDevice();
	const auto logicalDevice = gpuContext.LogicalDevice();

	std::vector<cof::UnlitColoredVertex> vertices
	{
		{ glm::vec3{0.0f, -0.5f, 0.0f}, glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }},
		{ glm::vec3{0.5f, 0.5f, 0.0f}, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f} },
		{ glm::vec3{-0.5f, 0.5f, 0.0f}, glm::vec4{0.0f, 0.0f, 1.0f, 1.0f} }
	};

	VkBufferCreateInfo bufferInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = sizeof(decltype(vertices)::value_type) * vertices.size(),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	VkDeviceSize bufferSize = sizeof(decltype(vertices)::value_type) * vertices.size();

	VkBuffer vertexBuffer, stagingBuffer;
	VkDeviceMemory vertexBufferMemory, stagingBufferMemory;
	createBuffer(gpuContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	createBuffer(gpuContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	void* vertexData;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferInfo.size, 0, &vertexData);
	memcpy(vertexData, vertices.data(), static_cast<size_t>(bufferInfo.size));
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	cof::CommandPool<VK_QUEUE_TRANSFER_BIT> transferCommandPool{ gpuContext, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT };

	VkCommandBufferAllocateInfo allocTransferCBufferInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = transferCommandPool.Handle(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1u
	};

	VkCommandBuffer transferCommandBuffer;
	errorCode = vkAllocateCommandBuffers(logicalDevice, &allocTransferCBufferInfo, &transferCommandBuffer);
	assert(errorCode == VK_SUCCESS);

	VkCommandBufferBeginInfo transferBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	vkBeginCommandBuffer(transferCommandBuffer, &transferBeginInfo);

	VkBufferCopy copyRegion
	{
		.size = bufferSize
	};

	VkBufferCopy copyRegions[]{ copyRegion };
	vkCmdCopyBuffer(transferCommandBuffer, stagingBuffer, vertexBuffer, static_cast<uint32_t>(std::size(copyRegions)), copyRegions);
	vkEndCommandBuffer(transferCommandBuffer);

	VkSubmitInfo submitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &transferCommandBuffer
	};

	VkQueue transferQueue;
	vkGetDeviceQueue(logicalDevice, gpuContext.QueueFamilyIndex<VK_QUEUE_TRANSFER_BIT>(), 0, &transferQueue);
	errorCode = vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE); //TODO add fence
	vkQueueWaitIdle(transferQueue);

	vkFreeCommandBuffers(logicalDevice, transferCommandPool.Handle(), 1, &transferCommandBuffer);
	vkDestroyBuffer(logicalDevice , stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);

	uint32_t presentQueueFamilyIndex{ std::numeric_limits<uint32_t>::max() };
	VkBool32 presentationSupported{ VK_FALSE };

	if (errorCode = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndices.graphics, surface, &presentationSupported); 
		VK_SUCCESS == errorCode && VK_TRUE == presentationSupported)
	{
		presentQueueFamilyIndex = queueFamilyIndices.graphics;
	}
	else if (errorCode = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndices.compute, surface, &presentationSupported);
		VK_SUCCESS == errorCode && VK_TRUE == presentationSupported)
	{
		presentQueueFamilyIndex = queueFamilyIndices.compute;
	}
	else if (errorCode = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndices.transfer, surface, &presentationSupported);
		VK_SUCCESS == errorCode && VK_TRUE == presentationSupported)
	{
		presentQueueFamilyIndex = queueFamilyIndices.transfer;
	}

	assert(presentQueueFamilyIndex != std::numeric_limits<uint32_t>::max());

	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	cof::Swapchain swapchain
	{
		gpuContext,
		surface,
		{ static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight) },
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
	};

	VkAttachmentDescription colorAttachment
	{
		.format = swapchain.ImageMetaData().format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR

	};

	VkAttachmentReference colorAttachmentRef
	{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass
	{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	VkSubpassDependency dependency
	{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	cof::RenderPass forwardGeometryPass{ logicalDevice, {colorAttachment}, {subpass}, {dependency} };

	cof::Shader triangleVertShader = cof::LoadShader(R"(D:\GameDev\Graphics\Vulkan\Nomad\Assets\Shaders\VBufferTriangle.vert.spv)", logicalDevice);
	cof::Shader triangleFragShader = cof::LoadShader(R"(D:\GameDev\Graphics\Vulkan\Nomad\Assets\Shaders\VBufferTriangle.frag.spv)", logicalDevice);

	VkPipelineShaderStageCreateInfo shaderStages[] = 
	{	
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = triangleVertShader.Handle(),
			.pName = "main"
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = triangleFragShader.Handle(),
			.pName = "main"
		}
	};

	VkVertexInputBindingDescription bindingDescription
	{
		.binding = 0,
		.stride = sizeof(decltype(vertices)::value_type),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};

	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions
	{
		VkVertexInputAttributeDescription
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(decltype(vertices)::value_type, position)
		},
		VkVertexInputAttributeDescription
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(decltype(vertices)::value_type, color)
		}
	};


	VkPipelineVertexInputStateCreateInfo vertexInputInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data()
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkViewport viewport
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(swapchain.ImageMetaData().extent.width),
		.height = static_cast<float>(swapchain.ImageMetaData().extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor
	{
		.offset = { 0, 0 },
		.extent = swapchain.ImageMetaData().extent
	};

	VkPipelineViewportStateCreateInfo viewportState
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	VkPipelineRasterizationStateCreateInfo rasterizer
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisampling
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment
	{
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo colorBlending
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};


	VkPipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
	};

	VkPipelineLayout pipelineLayout;
	errorCode = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout);

	VkGraphicsPipelineCreateInfo pipelineInfo
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlending,
		.pDynamicState = nullptr,
		.layout = pipelineLayout,
		.renderPass = forwardGeometryPass.Handle(),
		.subpass = 0,
	};

	VkPipeline graphicsPipeline;
	errorCode = vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
	assert(errorCode == VK_SUCCESS);

	cof::CommandPool<VK_QUEUE_GRAPHICS_BIT> graphicsCommandPool{ gpuContext, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };

	VkCommandBufferAllocateInfo allocCBufferInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = graphicsCommandPool.Handle(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1u
	};

	VkCommandBuffer graphicsCommandBuffer;

	errorCode = vkAllocateCommandBuffers(logicalDevice, &allocCBufferInfo, &graphicsCommandBuffer);
	assert(errorCode == VK_SUCCESS);

	cof::Semaphore imageAvailableSemaphore{ logicalDevice };
	cof::Semaphore renderingFinishedSemaphore{ logicalDevice };

	VkFence renderingFinishedFence;

	VkFenceCreateInfo fenceInfo
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	};

	errorCode = vkCreateFence(logicalDevice, &fenceInfo, nullptr, &renderingFinishedFence);
	assert(errorCode == VK_SUCCESS);

	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();
		uint32_t imageIndex = swapchain.AcquireNextImage(std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore.Handle(), VK_NULL_HANDLE);

		VkCommandBufferBeginInfo beginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};

		vkBeginCommandBuffer(graphicsCommandBuffer, &beginInfo);

		VkImageViewCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchain.Image(imageIndex),
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchain.ImageMetaData().format,
			.components = 
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY, 
				.b = VK_COMPONENT_SWIZZLE_IDENTITY, 
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = 
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkImageView imageView;
		vkCreateImageView(logicalDevice, &createInfo, nullptr, &imageView);

		VkFramebufferCreateInfo framebufferInfo
		{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = forwardGeometryPass.Handle(),
			.attachmentCount = 1,
			.pAttachments = &imageView,
			.width = swapchain.ImageMetaData().extent.width,
			.height = swapchain.ImageMetaData().extent.height,
			.layers = 1,
		};

		VkFramebuffer frameBuffer;
		vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &frameBuffer);

		static VkClearValue clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderPassBeginInfo renderPassInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = forwardGeometryPass.Handle(),
			.framebuffer = frameBuffer,
			.renderArea =
			{
				.offset = {0, 0},
				.extent = swapchain.ImageMetaData().extent
			},
			.clearValueCount = 1,
			.pClearValues = &clearColor
		};

		vkCmdBeginRenderPass(graphicsCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(graphicsCommandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdDraw(graphicsCommandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

		vkCmdEndRenderPass(graphicsCommandBuffer);
		errorCode = vkEndCommandBuffer(graphicsCommandBuffer);
		assert(errorCode == VK_SUCCESS);

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore.Handle() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { renderingFinishedSemaphore.Handle() };
		VkCommandBuffer commandBuffers[]{ graphicsCommandBuffer };

		VkSubmitInfo submitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = static_cast<uint32_t>(std::size(waitSemaphores)),
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = static_cast<uint32_t>(std::size(commandBuffers)),
			.pCommandBuffers = commandBuffers,
			.signalSemaphoreCount = static_cast<uint32_t>(std::size(signalSemaphores)),
			.pSignalSemaphores = signalSemaphores
		};
		
		VkQueue graphicsQueue;
		vkGetDeviceQueue(logicalDevice, gpuContext.QueueFamilyIndex<VK_QUEUE_GRAPHICS_BIT>(), 0, &graphicsQueue);
		errorCode = vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderingFinishedFence);

		
		VkFence waitFences[]{ renderingFinishedFence };
		const uint32_t numWaitFences{ static_cast<uint32_t>(std::size(waitFences)) };
		
		vkWaitForFences(logicalDevice, numWaitFences, waitFences, VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkResetFences(logicalDevice, numWaitFences, waitFences);

		VkSwapchainKHR swapchains[] = { swapchain.Handle() };

		VkPresentInfoKHR presentInfo
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = static_cast<uint32_t>(std::size(signalSemaphores)),
			.pWaitSemaphores = signalSemaphores,
			.swapchainCount = static_cast<uint32_t>(std::size(swapchains)),
			.pSwapchains = swapchains,
			.pImageIndices = &imageIndex
		};

		VkQueue presentQueue;
		vkGetDeviceQueue(logicalDevice, presentQueueFamilyIndex, 0, &presentQueue);

		vkQueuePresentKHR(presentQueue, &presentInfo);

		vkDestroyFramebuffer(logicalDevice, frameBuffer, nullptr);
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}

	vkDeviceWaitIdle(logicalDevice);

	vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
	vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);

	vkDestroyFence(logicalDevice, renderingFinishedFence, nullptr);
	vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);

	std::atexit([] 
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
			vkDestroyInstance(instance, nullptr); 
		});

	return 0;
}
