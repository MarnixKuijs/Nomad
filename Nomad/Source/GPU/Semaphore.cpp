#include "GPU/Semaphore.h"

#include <vulkan/vulkan_core.h>

#include <assert.h>

namespace cof
{
	Semaphore::Semaphore(VkDevice device, VkSemaphoreCreateFlags flags)
		: parent{ device }
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.flags = flags
		};

		[[maybe_unused]] VkResult errorCode = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &handle);
		assert(errorCode == VK_SUCCESS);
	}

	Semaphore::~Semaphore()
	{
		vkDestroySemaphore(parent, handle, nullptr);
	}
}