#pragma once
#include "GPU/PhysicalDevice.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

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

		const VkDevice LogicalDevice() const noexcept { return logicalDevice; }
		const VkPhysicalDevice PhysicalDevice() const noexcept { return physicalDevice; }
		const QueueFamilyIndices& QueueFamilyIndices() const noexcept { return queueFamilyIndices; }
	
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
}