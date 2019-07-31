#include "GPU/GPUContext.h"
#include "Utils/VulkanUtils.h"
#include "Platform/VulkanPlatform.h"

#include <array>
#include <string_view>
#include <assert.h>
#include <cstring>



#if defined(_DEBUG)
constexpr static std::array layers
{
	"VK_LAYER_LUNARG_standard_validation"
};
#else
constexpr static std::array<const char*, 0> layers{};
#endif

constexpr static std::array desiredInstanceExtensions{ VK_KHR_SURFACE_EXTENSION_NAME, COF_OS_SURFACE_EXTENIONS_NAME };

int main()
{
 	VkApplicationInfo appInfo 
 	{
 		VK_STRUCTURE_TYPE_APPLICATION_INFO,
 		nullptr,
 		"Nomad",
 		1u,
 		"LunarG SDK",
 		1u,
 		VK_API_VERSION_1_1
 	};

	VkResult result{ VK_RESULT_MAX_ENUM };

	uint32_t instanceExtensionsCount = 0;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);
	assert(result == VK_SUCCESS || instanceExtensionsCount != 0);


	std::vector<VkExtensionProperties> availableInstanceExtensions{ instanceExtensionsCount };
	result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, availableInstanceExtensions.data());
	assert(result == VK_SUCCESS);

	for (auto& desiredInstanceextension : desiredInstanceExtensions)
	{
		assert(IsExtensionSupported(availableInstanceExtensions, desiredInstanceextension));
	}

	VkInstanceCreateInfo instInfo
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

 	VkInstance instance;
 	result = vkCreateInstance(&instInfo, nullptr, &instance);
 	assert(result == VK_SUCCESS);


	uint64_t desiredFeaturesBitMask{ 1 | 1 << 1 | 1 << 2 | 1 << 3 };
	VkQueueFlags queueFlags{ VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT };
	std::vector desiredDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	cof::GPUContext device{ instance, desiredFeaturesBitMask, queueFlags, desiredDeviceExtensions };

 	vkDestroyInstance(instance, nullptr);

#ifdef VK_USE_PLATFORM_WIN32_KHR
	puts("windows");
#elif VK_USE_PLATFORM_XLIB_KHR
	puts("linux");
#endif

	return 0;
}
