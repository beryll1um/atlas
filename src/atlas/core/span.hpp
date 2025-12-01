#pragma once

#include <span>
#include <cstddef>

#include <atlas/core/type_traits.hpp>

namespace atl {

namespace detail {
template <class U, std::size_t N>
inline constexpr auto kExtentByteSize = N == std::dynamic_extent ? std::dynamic_extent : N * sizeof(U);
}  // namespace detail

template <ByteLike T, class U, std::size_t N>
[[nodiscard]] auto AsByteLike(std::span<U, N> span) noexcept {
	return std::span<const T, detail::kExtentByteSize<U, N>>{
		reinterpret_cast<const T*>(span.data()), span.size_bytes()
	};
}

}  // namespace atl

// vim: set ts=4 sw=4 noexpandtab:

