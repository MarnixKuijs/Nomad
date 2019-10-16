#pragma once
#include "GPU/GPUContext.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <assert.h>

namespace cof
{
	template<VkQueueFlagBits QueueType>
	struct CommandPool
	{
		CommandPool(VkCommandPoolCreateFlags flags = 0);
		~CommandPool() = default;
		const VkCommandPool Handle() const noexcept { return handle; }
		
		constexpr static VkQueueFlagBits type{ QueueType };

	private:
		VkCommandPool handle;
	};

	template<>
	inline CommandPool<VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT>::CommandPool(VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = flags,
			.queueFamilyIndex = GPUContext::QueueFamilyIndices().graphics
		};

		vkCreateCommandPool(GPUContext::LogicalDevice(), &poolInfo, nullptr, &handle);
	}

	template<>
	inline CommandPool<VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT>::CommandPool(VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = flags,
			.queueFamilyIndex = GPUContext::QueueFamilyIndices().compute
		};

		vkCreateCommandPool(GPUContext::LogicalDevice(), &poolInfo, nullptr, &handle);
	}

	template<>
	inline CommandPool<VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT>::CommandPool(VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = flags,
			.queueFamilyIndex = GPUContext::QueueFamilyIndices().transfer
		};

		vkCreateCommandPool(GPUContext::LogicalDevice(), &poolInfo, nullptr, &handle);
	}

}
