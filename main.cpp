#include "GPU/GPUContext.h"
#include "Graphics/Swapchain.h"
#include "Graphics/CommandPool.h"
#include "Graphics/Shader.h"
#include "Utils/VulkanUtils.h"

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

constexpr static VkApplicationInfo appInfo
{
	VK_STRUCTURE_TYPE_APPLICATION_INFO,
	nullptr,
	"Nomad",
	1u,
	"LunarG SDK",
	1u,
	VK_API_VERSION_1_1
};

#if defined(_DEBUG)
constexpr static std::array layers
{
	"VK_LAYER_LUNARG_standard_validation"
};
#else
constexpr static std::array<const char*, 0> layers{};
#endif


constexpr static uint64_t desiredFeaturesBitMask{ 1 | 1 << 1 | 1 << 2 | 1 << 3 };
constexpr static VkQueueFlags queueFlags{ VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT };

static std::vector<const char*> desiredInstanceExtensions{};
static std::vector<const char*> desiredDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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

	uint32_t instanceExtensionsCount = 0;
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
	
	const VkDevice logicalDevice = gpuContext.LogicalDevice();
	const VkPhysicalDevice physicalDevice = gpuContext.PhysicalDevice();
	const auto& queueFamilyIndices = gpuContext.QueueFamilyIndices();

	uint32_t presentQueueFamilyIndex{ std::numeric_limits<uint32_t>::max() };
	VkBool32 presentationSupported = VK_FALSE;

	if (errorCode = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndices.graphics, surface, &presentationSupported); 
		errorCode == VK_SUCCESS   && presentationSupported == VK_TRUE)
	{
		presentQueueFamilyIndex = queueFamilyIndices.graphics;
	}
	else if (errorCode = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndices.compute, surface, &presentationSupported);
		errorCode == VK_SUCCESS && presentationSupported == VK_TRUE)
	{
		presentQueueFamilyIndex = queueFamilyIndices.compute;
	}
	else if (errorCode = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndices.transfer, surface, &presentationSupported);
		errorCode == VK_SUCCESS && presentationSupported == VK_TRUE)
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

	VkSemaphoreCreateInfo semaphoreCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	
	VkSemaphore imageAvailableSemaphore, renderingFinishedSemaphore;
	errorCode = vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore);
	assert(errorCode == VK_SUCCESS);
	errorCode = vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &renderingFinishedSemaphore);
	assert(errorCode == VK_SUCCESS);

	[[maybe_unused]] uint32_t imageIndex = swapchain.AcquireNextImage(logicalDevice, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE);

	cof::Shader triangleVertSahder = cof::LoadShader(R"(D:\GameDev\Graphics\Vulkan\Nomad\Assets\Trangle.vert.spv)", logicalDevice);
	cof::Shader triangleFragShader = cof::LoadShader(R"(D:\GameDev\Graphics\Vulkan\Nomad\Assets\Trangle.frag.spv)", logicalDevice);

	cof::CommandPool<VK_QUEUE_GRAPHICS_BIT> graphicsCommandPool{};

	vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(logicalDevice, renderingFinishedSemaphore, nullptr);
	vkDestroyShaderModule(logicalDevice, triangleVertSahder.Handle(), nullptr);
	vkDestroyShaderModule(logicalDevice, triangleFragShader.Handle(), nullptr);
	vkDestroySwapchainKHR(logicalDevice, swapchain.Handle(), nullptr);
	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr); 

	return 0;
}
