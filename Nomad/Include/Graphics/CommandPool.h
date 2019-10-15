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
		CommandPool(const cof::GPUContext& device, VkCommandPoolCreateFlags flags = 0);
		~CommandPool();
		const VkCommandPool Handle() const noexcept { return handle; }
		constexpr static VkQueueFlagBits type{ QueueType };

	private:
		VkCommandPool handle;
		const VkDevice parent;
	};

	template<>
	inline CommandPool<VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT>::CommandPool(const cof::GPUContext& gpuContext, VkCommandPoolCreateFlags flags)
		: parent(gpuContext.LogicalDevice())
	{
		VkCommandPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = flags,
			.queueFamilyIndex = gpuContext.QueueFamilyIndices().graphics
		};

		vkCreateCommandPool(gpuContext.LogicalDevice(), &poolInfo, nullptr, &handle);
	}

	template<>
	inline CommandPool<VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT>::CommandPool(const cof::GPUContext& gpuContext, VkCommandPoolCreateFlags flags)
		: parent(gpuContext.LogicalDevice())
	{
		VkCommandPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = flags,
			.queueFamilyIndex = gpuContext.QueueFamilyIndices().compute
		};

		vkCreateCommandPool(gpuContext.LogicalDevice(), &poolInfo, nullptr, &handle);
	}

	template<>
	inline CommandPool<VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT>::CommandPool(const cof::GPUContext& gpuContext, VkCommandPoolCreateFlags flags)
		: parent(gpuContext.LogicalDevice())
	{
		VkCommandPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = flags,
			.queueFamilyIndex = gpuContext.QueueFamilyIndices().transfer
		};

		vkCreateCommandPool(gpuContext.LogicalDevice(), &poolInfo, nullptr, &handle);
	}

	template<VkQueueFlagBits QueueType>
	inline CommandPool<QueueType>::~CommandPool()
	{
		vkDestroyCommandPool(parent, handle, nullptr);
		handle = VK_NULL_HANDLE;
	}

}
