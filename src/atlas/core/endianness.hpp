#pragma once

#include <bit>
#include <concepts>

namespace atl {

template <std::integral T>
T ToBigEndian(T x) {
	if constexpr (std::endian::native == std::endian::little) {
		return std::byteswap(x);
	}
	return x;
}

}  // namespace atl

// vim: set ts=4 sw=4 noexpandtab:

