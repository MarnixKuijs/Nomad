#pragma once
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>
#include <assert.h>
namespace cof
{

	struct GPUContext
	{
	private:
		struct QueueFamilyIndices;

	public:
		//TODO change desiredExtensions to span
		GPUContext(	const VkInstance instance, 
					const uint64_t desiredFeaturesBitMask, 
					const VkQueueFlags desiredQueueFamilies, 
					const std::vector<const char*>& desiredExtensions);
		~GPUContext();
		GPUContext(const GPUContext& other) = delete;
		GPUContext& operator=(const GPUContext& other) = delete;
		GPUContext(GPUContext&& other) = delete;
		GPUContext& operator=(GPUContext&& other) = delete;

		VkDevice LogicalDevice() const noexcept { return logicalDevice; }
		VkPhysicalDevice PhysicalDevice() const noexcept { return physicalDevice; }
		const QueueFamilyIndices& QueueFamilyIndices() const noexcept { return queueFamilyIndices; }

		template<VkQueueFlagBits QueueType>
		uint32_t QueueFamilyIndex() const noexcept;
	
	private:

		VkDevice logicalDevice;
		VkPhysicalDevice physicalDevice;

		struct QueueFamilyIndices
		{
			uint32_t graphics{ std::numeric_limits<uint32_t>::max() };
			uint32_t compute{ std::numeric_limits<uint32_t>::max() };
			uint32_t transfer{ std::numeric_limits<uint32_t>::max() };
		} queueFamilyIndices;
	};

	template<VkQueueFlagBits QueueType>
	inline uint32_t GPUContext::QueueFamilyIndex() const noexcept
	{
		if constexpr (QueueType == VK_QUEUE_GRAPHICS_BIT)
		{
			return queueFamilyIndices.graphics;
		}
		else if constexpr (QueueType == VK_QUEUE_COMPUTE_BIT)
		{
			return queueFamilyIndices.compute;
		}
		else if constexpr (QueueType == VK_QUEUE_TRANSFER_BIT)
		{
			return queueFamilyIndices.transfer;
		}
		else
		{
			assert(false);
			return std::numeric_limits<uint32_t>::max();
		}
	}
}