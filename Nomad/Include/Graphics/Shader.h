#pragma once
#include <vulkan/vulkan_core.h>

#include <vector>
#include <cstddef>
#include <filesystem>

namespace cof
{
	struct Shader
	{
		Shader(const std::vector<std::byte>& buffer);
		~Shader();
	private:
		VkShaderModule shaderModule;
	};

	Shader LoadShader(std::filesystem::path&& shaderPath);
	Shader LoadShader(const std::filesystem::path& shaderPath);
}
