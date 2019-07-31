#pragma once
#include <array>
#include <cstring>
#include <type_traits>

namespace cof
{
	//C++20 function fill 
	template <class To, class From>
	inline To bit_cast(const From& src) noexcept 
	{
		static_assert(sizeof(To) == sizeof(From));
		static_assert(std::is_trivially_copyable_v<From>);
		static_assert(std::is_trivial_v<To>);

		To dst{};
		std::memcpy(&dst, &src, sizeof(To));
		return dst;
	}

	template <typename Member, typename T>
	inline auto ArrayCast(T const& value) noexcept 
	{
		static_assert(sizeof(T) % sizeof(Member) == 0);

		return cof::bit_cast<std::array<Member, sizeof(T) / sizeof(Member)>>(value);
	}

	template <typename T, typename Member>
	inline auto ReverseArrayCast(std::array<Member, sizeof(T) / sizeof(Member)> const& arr) noexcept
	{
		static_assert(sizeof(T) % sizeof(Member) == 0);

		return cof::bit_cast<T>(arr);
	}
}