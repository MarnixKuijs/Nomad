#include <string_view>
#include <vulkan/vulkan_core.h>

inline bool IsExtensionSupported(const std::vector<VkExtensionProperties>& availableExtensions, std::string_view desiredExtension)
{
	for (auto& availableExtension : availableExtensions)
	{
		if (desiredExtension == std::string_view{ availableExtension.extensionName })
		{
			return true;
		}
	}
	return false;
}
