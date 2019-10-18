#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
namespace cof
{
	struct GPUContext;
	class Swapchain
	{
	private:
		struct ImageMetaData;
	public:
		Swapchain
		(	
			const cof::GPUContext& gpuContext, 
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

		VkSwapchainKHR Handle() const noexcept { return handle; }
		const ImageMetaData& ImageMetaData() const noexcept { return imageMetaData; }
		const std::vector<VkImage>& Images() const noexcept{ return images; }
		VkImage Image(uint32_t index) const noexcept { return images[index]; }

		const uint32_t AcquireNextImage
		(
			uint64_t timeout,
			VkSemaphore semaphore,
			VkFence fence
		);

	private:
		VkSwapchainKHR handle;
		const VkDevice parent;

		struct ImageMetaData
		{
			VkFormat format;
			VkExtent2D extent;
		} imageMetaData;

		std::vector<VkImage> images;
	};
}
