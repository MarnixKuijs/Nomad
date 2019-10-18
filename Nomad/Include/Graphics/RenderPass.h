#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
namespace cof
{
	struct RenderPass
	{
		//TODO change vector to span
		RenderPass(	VkDevice device, 
					const std::vector<VkAttachmentDescription>& attachments, 
					const std::vector<VkSubpassDescription>& subpasses,
					const std::vector<VkSubpassDependency>& subpassDependencies);
		~RenderPass();

		VkRenderPass Handle() const noexcept { return handle; }

	private:
		VkRenderPass handle;
		const VkDevice parent;
	};
}