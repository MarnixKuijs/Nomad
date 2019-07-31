#pragma once
#include <vulkan/vulkan_core.h>

#include <utility>
#include <cstdint>

namespace cof
{
	struct PhysicalDevice
	{
		friend struct GPUContext;
		friend PhysicalDevice RequestPhysicalDevice(VkInstance instance, uint64_t desiredFeaturesBitMask);

		~PhysicalDevice()
		{
			handle = VK_NULL_HANDLE;
		}

	private:
		PhysicalDevice
		(
			VkPhysicalDevice&& physicalDeviceHandle,
			VkPhysicalDeviceFeatures&& deviceFeatures,
			VkPhysicalDeviceProperties&& deviceProperties
		) 
			: handle{ std::move(physicalDeviceHandle) }
			, deviceFeatures{ std::move(deviceFeatures) }
			, deviceProperties{ std::move(deviceProperties) }
		{}

		PhysicalDevice(const PhysicalDevice& other) = delete;
		PhysicalDevice& operator=(const PhysicalDevice& other) = delete;

		PhysicalDevice(PhysicalDevice&& other) noexcept
		{
			handle = other.handle;
			deviceFeatures = other.deviceFeatures;
			deviceProperties = other.deviceProperties;

			other.handle = VK_NULL_HANDLE;
		}
		PhysicalDevice& operator=(PhysicalDevice&& other) noexcept
		{
			if (this != &other)
			{
				handle = other.handle;
				deviceFeatures = other.deviceFeatures;
				deviceProperties = other.deviceProperties;

				other.handle = VK_NULL_HANDLE;
			}

			return *this;
		}

		VkPhysicalDevice handle;
		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceProperties deviceProperties;
	};
}