#pragma once
#include <vulkan/vulkan_core.h>

namespace cof
{
	struct Semaphore
	{
		Semaphore(VkDevice device, VkSemaphoreCreateFlags flags = 0);
		~Semaphore();

		Semaphore(const Semaphore& other) = delete;
		Semaphore& operator=(const Semaphore& other) = delete;
		Semaphore(Semaphore&& other) = delete;
		Semaphore& operator=(Semaphore&& other) = delete;

		VkSemaphore Handle() const noexcept { return handle; }
	private:
		VkSemaphore handle;
		const VkDevice parent;
	};
}