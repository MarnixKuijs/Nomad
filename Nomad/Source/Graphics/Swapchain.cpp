#include <Graphics/Swapchain.h>
#include <GPU/GPUContext.h>

#include <assert.h>
#include <cstdlib>
#include <algorithm>

namespace cof
{
	Swapchain::Swapchain(
		const cof::GPUContext& gpuContext,
		const VkSurfaceKHR surface,
		const VkExtent2D desiredImageSize,
		const VkPresentModeKHR desiredPresentMode,
		const VkImageUsageFlags dsiredUsages,
		const VkSurfaceTransformFlagBitsKHR desiredTransform,
		const VkSurfaceFormatKHR desiredSurfaceFormat)
		: parent { gpuContext.LogicalDevice() }
	{
		VkResult errorCode{ VK_RESULT_MAX_ENUM };
		uint32_t presentModesCount{0};
		errorCode = vkGetPhysicalDeviceSurfacePresentModesKHR(gpuContext.PhysicalDevice(), surface, &presentModesCount, nullptr);
		assert(errorCode == VK_SUCCESS || presentModesCount != 0 );

		std::vector<VkPresentModeKHR> presentModes{ presentModesCount };
		errorCode = vkGetPhysicalDeviceSurfacePresentModesKHR(gpuContext.PhysicalDevice(), surface, &presentModesCount, presentModes.data());
		assert(errorCode == VK_SUCCESS);

		VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };
		if (desiredPresentMode != VK_PRESENT_MODE_FIFO_KHR)
		{
			for (auto& presentModeCanidate : presentModes)
			{
				if (presentModeCanidate == desiredPresentMode)
				{
					presentMode = desiredPresentMode;
					break;
				}
			}
		}

		uint32_t formatsCount{0};
		errorCode = vkGetPhysicalDeviceSurfaceFormatsKHR(gpuContext.PhysicalDevice(), surface, &formatsCount, nullptr);
		assert(errorCode == VK_SUCCESS && formatsCount != 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats{ formatsCount };
		errorCode = vkGetPhysicalDeviceSurfaceFormatsKHR(gpuContext.PhysicalDevice(), surface, &formatsCount, surfaceFormats.data());
		assert(errorCode == VK_SUCCESS);

		VkFormat imageFormat{ surfaceFormats[0].format };
		VkColorSpaceKHR colorSpace{ surfaceFormats[0].colorSpace };

		if ( surfaceFormats.size() == 1 &&  surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			puts("Desired image format and color space selected");
			imageFormat = desiredSurfaceFormat.format;
			colorSpace = desiredSurfaceFormat.colorSpace;
		}
		else
		{
			for (auto& surfaceFormat : surfaceFormats)
			{
				if (desiredSurfaceFormat.format == surfaceFormat.format && desiredSurfaceFormat.colorSpace == surfaceFormat.colorSpace)
				{
					puts("Desired image format and color space selected");
					imageFormat = desiredSurfaceFormat.format;
					colorSpace = desiredSurfaceFormat.colorSpace;
					break;
				}
			}
		}

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpuContext.PhysicalDevice(), surface, &surfaceCapabilities);
		assert(VK_SUCCESS == result);

		VkExtent2D imageExtent{ surfaceCapabilities.currentExtent };
		if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
		{
			puts("Desired Image Size selected");
			imageExtent.width = std::clamp(desiredImageSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			imageExtent.height = std::clamp(desiredImageSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
		{
			imageCount = surfaceCapabilities.maxImageCount;
		}

		VkImageUsageFlags imageUsage = dsiredUsages & surfaceCapabilities.supportedUsageFlags;
		assert(imageUsage == dsiredUsages);

		VkSurfaceTransformFlagBitsKHR imageTransform{ surfaceCapabilities.currentTransform };
		
		if ((surfaceCapabilities.supportedTransforms & desiredTransform) == static_cast<uint32_t>(desiredTransform))
		{
			imageTransform = desiredTransform;
		}

		VkSwapchainCreateInfoKHR swapchainCreateInfo
		{
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  
			nullptr,                                     
			0,                                           
			surface,            
			imageCount,                            
			imageFormat,								 
			colorSpace,							 
			imageExtent,                                  
			1,                                           
			imageUsage,                                 
			VK_SHARING_MODE_EXCLUSIVE,   //TODO make SHRIND_MODE_CONCURRENT available                
			0,                                           
			nullptr,                                     
			imageTransform,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,           
			presentMode,                    
			VK_TRUE,                                     
			VK_NULL_HANDLE //TODO recreation of swapchain                                
		};

		errorCode = vkCreateSwapchainKHR(gpuContext.LogicalDevice(), &swapchainCreateInfo, nullptr, &handle);
		assert(errorCode == VK_SUCCESS);

		imageMetaData.extent = imageExtent;
		imageMetaData.format = imageFormat;

		vkGetSwapchainImagesKHR(gpuContext.LogicalDevice(), handle, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(gpuContext.LogicalDevice(), handle, &imageCount, images.data());
	}

	Swapchain::~Swapchain()
	{
		vkDestroySwapchainKHR(parent, handle, nullptr);
	}

	const uint32_t Swapchain::AcquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence)
	{
		uint32_t imageIndex;
		VkResult spawchainStatus = vkAcquireNextImageKHR(parent, handle, timeout, semaphore, fence, &imageIndex);
		switch (spawchainStatus) //TODO handle different cases
		{
		case VK_SUCCESS:
			break;
		case VK_SUBOPTIMAL_KHR:
			assert(false);
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			assert(false);
			break;
		default:
			assert(false);
			break;
		}

		return imageIndex;
	}
}
