#pragma once
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

namespace cof
{
	struct BaseUnlitVertex
	{
		glm::vec3 position;
	};

	struct UnlitColoredVertex : BaseUnlitVertex
	{
		glm::vec4 color;
	};

	struct UnlitTexturedVertex : BaseUnlitVertex
	{
		glm::vec2 texCoord;
	};

	struct BaseLitVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
	};
}
