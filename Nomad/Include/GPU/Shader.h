#pragma once
#include <vulkan/vulkan_core.h>

#include <vector>
#include <cstddef>
#include <filesystem>

namespace cof
{
	struct Shader
	{
		Shader(const VkDevice device, const std::vector<std::byte>& buffer);
		~Shader();

		VkShaderModule Handle() const noexcept { return handle; }
	private:
		VkShaderModule handle;
		const VkDevice parent;
	};

	Shader LoadShader(std::filesystem::path&& shaderPath, const VkDevice device);
	Shader LoadShader(const std::filesystem::path& shaderPath, const VkDevice device);
}
