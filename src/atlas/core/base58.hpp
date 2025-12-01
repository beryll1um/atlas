#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <string_view>

#include <atlas/core/type_traits.hpp>

namespace atl::base58 {

namespace detail {
inline constinit std::int8_t kBase58CharToNum[128] = {
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1, // '1'..'9'
	-1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1, // 'A'..'Z' (I skipped)
	22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1, // 'a'..'z' (l skipped)
	-1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
	47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1
};
static_assert(sizeof(kBase58CharToNum) == 128, "The size of kBase58CharToNum must be 128 bytes");

// The length of the output depends on the conversion factor from Base58 to Base256,
// which is essentially log2(58) / log2(256) rounded up.
inline constinit float kBase58to256ConvFactor = 0.7322476243909465;
}  // namespace detail

template <ByteLike T>
[[nodiscard]] std::vector<T> FromString(std::string_view input) {
	// Counts leading zeros (which are represented as the character "1" in Base58 encoding system).
	std::size_t leading_zeros = 0;
	while (!input.empty() && input.front() == '1') {
		++leading_zeros;
		input.remove_prefix(1);
	}
	if (input.empty()) {
		// The whole number is zero, so we can approach the fast path.
		return std::vector<T>(leading_zeros, T{});
	}
	// This can be simply understood as follows: to encode any Base58 into Base256 (byte) form,
	// we need that many times fewer characters.
	// Leading zeros should always be treated as independent bytes,
	// since they are a prefix according to the notation.
	std::vector<T> output;
	output.reserve(leading_zeros + input.size() * detail::kBase58to256ConvFactor + 1);
	output.resize(leading_zeros);
	// In a nutshell it's just bigint arithmetic for this specific Base58 to Base256 case.
	for (unsigned char b58char : input) {
		// Shrinks the mapping array. The performance impact is negligible because invalid Base58 strings are rare,
		// but it saves just under 128 bytes of CPU cache.
		if (b58char & 0x80) [[unlikely]] {
			throw std::runtime_error("Invalid base58 character during decoding (1)");
		}
		// Convert Base58 character to the corresponding number (0..57).
		auto b58num = detail::kBase58CharToNum[b58char];
		if (b58num == -1) [[unlikely]] {
			throw std::runtime_error("Invalid base58 character during decoding (2)");
		}
		// Increment the order of each bigint limb, place its
		// least significant byte in the actual position,
		// and move reminder forward.
		auto remainder = static_cast<std::uint64_t>(b58num);
		for (auto i = leading_zeros; i < output.size(); ++i) {
			remainder += static_cast<std::uint8_t>(output[i]) * 58;
			output[i] = static_cast<T>(remainder);
			remainder >>= 8;
		}
		while (remainder) {
			output.push_back(static_cast<T>(remainder));
			remainder >>= 8;
		}
	}
	std::reverse(output.begin() + leading_zeros, output.end());
	return output;
}

}  // namespace atl::base58

// vim: set ts=4 sw=4 noexpandtab:

