#include "GPU/GPUContext.h"
#include "Utils/Utils.h"
#include "Utils/VulkanUtils.h"

#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <limits>

namespace cof
{
	PhysicalDevice RequestPhysicalDevice(VkInstance instance, ::uint64_t desiredFeaturesBitMask);
	const uint32_t GetGraphicsQueueFamilyIndex(const std::vector<VkQueueFamilyProperties>& queueFamilies);
	const uint32_t GetComputeQueueFamilyIndex(const std::vector<VkQueueFamilyProperties>& queueFamilies);
	const uint32_t GetTransferQueueFamilyIndex(const std::vector<VkQueueFamilyProperties>& queueFamilies);

	
	GPUContext::GPUContext(	const VkInstance instance, 
							const uint64_t desiredFeaturesBitMask, 
							const VkQueueFlags desiredQueueFamilies, 
							const std::vector<const char*>& desiredExtensions)
		: physicalDevice{ RequestPhysicalDevice(instance, desiredFeaturesBitMask) }
	{

		uint32_t queueFamiliesCount = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.handle, &queueFamiliesCount, nullptr);
		assert(queueFamiliesCount != 0);

		std::vector<VkQueueFamilyProperties> queueFamilies{ queueFamiliesCount };
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.handle, &queueFamiliesCount, queueFamilies.data());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		constexpr float defaultQueuePriority{ 1.0f };

		if	((desiredQueueFamilies & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{	
			queueFamilyIndices.graphics = GetGraphicsQueueFamilyIndex(queueFamilies);

			queueCreateInfos.emplace_back
			(
				VkDeviceQueueCreateInfo
				{
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					nullptr,
					0,
					queueFamilyIndices.graphics,
					1u,
					&defaultQueuePriority
				}
			);
		}

		if	((desiredQueueFamilies & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
		{	
			queueFamilyIndices.compute = GetComputeQueueFamilyIndex(queueFamilies);
			if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
			{
				queueCreateInfos.emplace_back
				(
					VkDeviceQueueCreateInfo
					{
						VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
						nullptr,
						0,
						queueFamilyIndices.compute,
						1u,
						&defaultQueuePriority
					}
				);
			}
		}

		if	((desiredQueueFamilies & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
		{
			queueFamilyIndices.transfer = GetTransferQueueFamilyIndex(queueFamilies);
			if (queueFamilyIndices.transfer != queueFamilyIndices.graphics && queueFamilyIndices.transfer != queueFamilyIndices.compute)
			{
				queueCreateInfos.emplace_back
				(
					VkDeviceQueueCreateInfo
					{
						VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
						nullptr,
						0,
						queueFamilyIndices.transfer,
						1u,
						&defaultQueuePriority
					}
				);
			}
		}

		std::vector<VkExtensionProperties> availableDeviceExtensions;

		uint32_t extensionsCount = 0;
		VkResult result;
		result = vkEnumerateDeviceExtensionProperties(physicalDevice.handle, nullptr, &extensionsCount, nullptr);
		assert(result == VK_SUCCESS && extensionsCount != 0);

		availableDeviceExtensions.resize(extensionsCount);
		result = vkEnumerateDeviceExtensionProperties(physicalDevice.handle, nullptr, &extensionsCount, availableDeviceExtensions.data());
		assert(result == VK_SUCCESS);

		for (auto& desiredExtension : desiredExtensions)
		{
			assert(IsExtensionSupported(availableDeviceExtensions, desiredExtension));
		}

		VkDeviceCreateInfo deviceCreateInfo
		{
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,							
			nullptr,														
			0,																
			static_cast<uint32_t>(queueCreateInfos.size()),
			queueCreateInfos.data(),
			0,																
			nullptr,														
			static_cast<uint32_t>(desiredExtensions.size()),		
			desiredExtensions.data(),
			&physicalDevice.deviceFeatures
		};

		result = vkCreateDevice(physicalDevice.handle, &deviceCreateInfo, nullptr, &logicalDevice);
		assert(result == VK_SUCCESS || logicalDevice != VK_NULL_HANDLE);
	}
	
	GPUContext::~GPUContext()
	{
	}

	PhysicalDevice RequestPhysicalDevice(VkInstance instance, uint64_t desiredFeaturesBitMask)
	{
		uint32_t deviceCount = 0;
		VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		assert(result == VK_SUCCESS && deviceCount != 0);

		std::vector<VkPhysicalDevice> availabePhysicalDevices{ deviceCount };
		std::vector<VkPhysicalDeviceFeatures> availablePhysicalDeviceFeatures{ deviceCount };
		std::vector<VkPhysicalDeviceProperties> availablePhysicalDeviceProperties{ deviceCount };

		result = vkEnumeratePhysicalDevices(instance, &deviceCount, availabePhysicalDevices.data());
		assert(result == VK_SUCCESS);

		std::vector<VkPhysicalDevice*> physicalDeviceCandidates{ deviceCount };
		std::vector<VkPhysicalDeviceFeatures*> physicalDeviceCandidatesFeatures{ deviceCount };
		std::vector<VkPhysicalDeviceProperties*> physicalDeviceCandidatesProperties{ deviceCount };

		for (size_t deviceIndex{}; deviceIndex < availabePhysicalDevices.size(); ++deviceIndex)
		{
			VkPhysicalDevice& physicalDeviceCandidate = availabePhysicalDevices[deviceIndex];
			VkPhysicalDeviceFeatures& physicalDeviceCandidateFeatures = availablePhysicalDeviceFeatures[deviceIndex];
			VkPhysicalDeviceProperties& physicalDeviceCandidateProperties = availablePhysicalDeviceProperties[deviceIndex];

			vkGetPhysicalDeviceFeatures(physicalDeviceCandidate, &physicalDeviceCandidateFeatures);
			vkGetPhysicalDeviceProperties(physicalDeviceCandidate, &physicalDeviceCandidateProperties);

			uint64_t deviceFeaturesBitMask{};
			auto deviceFeaturesArray = cof::ArrayCast<VkBool32>(physicalDeviceCandidateFeatures);

			for (std::size_t currentBit{}; currentBit < deviceFeaturesArray.size(); ++currentBit)
			{
				deviceFeaturesBitMask |= static_cast<uint64_t>(deviceFeaturesArray[currentBit]) << currentBit;
			}

			if ((deviceFeaturesBitMask & desiredFeaturesBitMask) != desiredFeaturesBitMask)
			{
				printf("Not all desired features were suppported by the following device: %s \n", physicalDeviceCandidateProperties.deviceName);
				continue;
			}

			physicalDeviceCandidates[deviceIndex] = &physicalDeviceCandidate;
			physicalDeviceCandidatesFeatures[deviceIndex] = &physicalDeviceCandidateFeatures;
			physicalDeviceCandidatesProperties[deviceIndex] = &physicalDeviceCandidateProperties;
		}

		assert(physicalDeviceCandidates.size() != 0);

		VkPhysicalDevice& selectedDevice = *physicalDeviceCandidates[0];
		VkPhysicalDeviceProperties& selectedDeviceProperties = *physicalDeviceCandidatesProperties[0];

		auto deviceFeaturesArray = cof::ArrayCast<VkBool32>(*physicalDeviceCandidatesFeatures[0]);
		for (size_t currentBit{}; currentBit < deviceFeaturesArray.size(); ++currentBit)
		{
			constexpr static uint64_t maskChecker{ 1 };
			deviceFeaturesArray[currentBit] = (desiredFeaturesBitMask >> currentBit) & maskChecker;
		}

		VkPhysicalDeviceFeatures selectedDeviceFeatures = cof::ReverseArrayCast<VkPhysicalDeviceFeatures>(deviceFeaturesArray);

		return cof::PhysicalDevice{ std::move(selectedDevice), std::move(selectedDeviceFeatures), std::move(selectedDeviceProperties) };
	}

	const uint32_t GetGraphicsQueueFamilyIndex(const std::vector<VkQueueFamilyProperties>& queueFamilies)
	{
		for (size_t i{}; i < queueFamilies.size(); ++i)
		{
			if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
			{
				return static_cast<uint32_t>(i);
			}
		}
		assert(false);
		return 0;
	}

	const uint32_t GetComputeQueueFamilyIndex(const std::vector<VkQueueFamilyProperties>& queueFamilies)
	{
		for (size_t i{}; i < queueFamilies.size(); ++i)
		{
			if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT &&
				(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
			{
				return static_cast<uint32_t>(i);
			}
		}

		for (size_t i{}; i < queueFamilies.size(); ++i)
		{
			if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1)
			{
				return static_cast<uint32_t>(i);
			}
		}

		assert(false);
		return 0;
	}

	const uint32_t GetTransferQueueFamilyIndex(const std::vector<VkQueueFamilyProperties>& queueFamilies)
	{
		for (size_t i{}; i < queueFamilies.size(); ++i)
		{
			if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT &&
				(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
				(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)
			{
				return static_cast<uint32_t>(i);
			}
		}

		for (size_t i{}; i < queueFamilies.size(); ++i)
		{
			if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1)
			{
				return static_cast<uint32_t>(i);
			}
		}
		assert(false);
		return 0;
	}
}
