#pragma once

#include <array>
#include <ranges>
#include <type_traits>

namespace atl {

enum class BufferStreamDirection { kForward, kBackward };

template <class T, BufferStreamDirection D = BufferStreamDirection::kForward>
	requires (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>)
class BufferStreamUnsafe final {
public:
	using value_type = T;
	using size_type = std::size_t;
	using pointer = value_type*;

	BufferStreamUnsafe(pointer ptr) : buf_{ptr} {}

	template <std::size_t N>
	BufferStreamUnsafe(std::array<T, N>& buf) : buf_{buf.data()} {
		if constexpr (D == BufferStreamDirection::kBackward) {
			buf_ += buf.size();
		}
	}

	template <std::ranges::contiguous_range R>
		requires std::same_as<std::remove_cvref_t<std::ranges::range_value_t<R>>, T>
	void write(R const& r) {
		if constexpr (D == BufferStreamDirection::kForward) {
			buf_ = std::ranges::copy(r, buf_).out;
		} else {
			buf_ = std::ranges::copy_backward(r, buf_).out;
		}
	}

private:
	pointer buf_;
};

}  // namespace atl

// vim: set ts=4 sw=4 noexpandtab:

