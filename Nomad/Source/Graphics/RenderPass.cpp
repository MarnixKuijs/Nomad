#include "Graphics/RenderPass.h"
#include <vulkan/vulkan_core.h>
#include <vector>
namespace cof
{
	RenderPass::RenderPass(	VkDevice device, 
							const std::vector<VkAttachmentDescription>& attachments, 
							const std::vector<VkSubpassDescription>& subpasses,
							const std::vector<VkSubpassDependency>& subpassDependencies)
		: parent{device}
	{


		VkRenderPassCreateInfo renderPassInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = static_cast<uint32_t>(subpasses.size()),
			.pSubpasses = subpasses.data(),
			.dependencyCount = static_cast<uint32_t>(subpassDependencies.size()),
			.pDependencies = subpassDependencies.data()
		};

		vkCreateRenderPass(device, &renderPassInfo, nullptr, &handle);
	}

	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(parent, handle, nullptr);
		handle = VK_NULL_HANDLE;
	}
}

