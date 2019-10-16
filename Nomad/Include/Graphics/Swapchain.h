#pragma once
#include <vulkan/vulkan_core.h>

namespace cof
{
	struct GPUContext;
	class Swapchain
	{
	public:
		Swapchain
		(	
			const VkSurfaceKHR surface, 
			const VkExtent2D desiredImageSize,
			const VkPresentModeKHR desiredPresentMode,	
			const VkImageUsageFlags dsiredUsages, 
			const VkSurfaceTransformFlagBitsKHR desiredTransform, 
			const VkSurfaceFormatKHR desiredSurfaceFormat
		);

		~Swapchain();

		Swapchain(const Swapchain& other) = delete;
		Swapchain& operator=(const Swapchain& other) = delete;
		Swapchain(Swapchain&& other) = delete;
		Swapchain& operator=(Swapchain&& other) = delete;

		const VkSwapchainKHR Handle() const noexcept { return handle; }

		const uint32_t AcquireNextImage
		(
			uint64_t timeout,
			VkSemaphore semaphore,
			VkFence fence
		);

	private:
		VkSwapchainKHR handle;
	};
}
