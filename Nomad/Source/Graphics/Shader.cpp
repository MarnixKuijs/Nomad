#include "Graphics/Shader.h"
#include "GPU/GPUContext.h"
#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <fstream>
#include <assert.h>
namespace cof
{
	Shader::Shader(const std::vector<std::byte>& buffer)
	{
		VkShaderModuleCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = buffer.size(),
			.pCode = reinterpret_cast<const uint32_t*>(buffer.data()) //TODO get rid of UB
		};

		VkResult errorCode = vkCreateShaderModule(cof::GPUContext::LogicalDevice(), &createInfo, nullptr, &shaderModule);
		assert(errorCode == VK_SUCCESS);
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(cof::GPUContext::LogicalDevice(), shaderModule, nullptr);
		shaderModule = VK_NULL_HANDLE;
	}

	Shader LoadShader(std::filesystem::path&& shaderPath)
	{
		std::ifstream shaderFile{ shaderPath, std::ios::ate | std::ios::binary };

		assert(shaderFile.is_open());

		size_t fileSize = static_cast<size_t>(shaderFile.tellg());
		std::vector<std::byte> buffer{ fileSize };

		shaderFile.seekg(0);
		shaderFile.read(reinterpret_cast<char*>(buffer.data()), fileSize); //Maybe UB?
		return Shader{ buffer };
	}

	Shader cof::LoadShader( const std::filesystem::path& shaderPath)
	{
		std::ifstream shaderFile{ shaderPath, std::ios::ate | std::ios::binary };

		assert(shaderFile.is_open());

		size_t fileSize = static_cast<size_t>(shaderFile.tellg());
		std::vector<std::byte> buffer{ fileSize };

		shaderFile.seekg(0);
		shaderFile.read(reinterpret_cast<char*>(buffer.data()), fileSize); //Maybe UB?
		return Shader{ buffer };
	}

}