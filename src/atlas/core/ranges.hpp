#pragma once

#include <ranges>
#include <cstddef>
#include <utility>
#include <iterator>
#include <algorithm>

namespace atl::view {

template <class R>
	requires std::ranges::view<R> && std::ranges::sized_range<R> && std::ranges::forward_range<R>
class RoundRobinView : public std::ranges::view_interface<RoundRobinView<R>>
{
private:
	template <class B>
	class Iterator {
	public:
		using base_iter = std::ranges::iterator_t<B>;
		using value_type = std::ranges::subrange<base_iter>;
		using iterator_concept = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using size_type = std::size_t;

		constexpr Iterator(base_iter iter, size_type elements, size_type partitions) noexcept :
			iter_{iter},
			elements_{elements},
			partitions_{partitions}
		{}

		constexpr value_type operator*() const {
			return {iter_, std::ranges::next(iter_, elements_ / partitions_)};
		}

		constexpr Iterator& operator++() {
			const auto size = elements_ / partitions_;
			iter_ = std::ranges::next(iter_, size);
			elements_ -= size;
			partitions_--;
			return *this;
		}

		constexpr Iterator operator++(int) {
			Iterator tmp = *this;
			++*this;
			return tmp;
		}

		friend constexpr bool operator==(const Iterator& lhs, const Iterator& rhs) = default;

		constexpr bool operator==(std::default_sentinel_t) const noexcept {
			return partitions_ == 0;
		}

	private:
		base_iter iter_{};
		size_type elements_{};
		size_type partitions_{};
	};

public:
	using iterator = Iterator<R>;
	using const_iterator = Iterator<const R>;
	using size_type = std::size_t;

	constexpr RoundRobinView(R base, std::size_t partitions) :
		base_(std::move(base)),
		partitions_(partitions)
	{}

	constexpr iterator begin() {
		const auto total = static_cast<size_type>(std::ranges::size(base_));
		return iterator{std::ranges::begin(base_), total, std::min(partitions_, total)};
	}

	constexpr const_iterator begin() const {
		const auto total = static_cast<size_type>(std::ranges::size(base_));
		return const_iterator{std::ranges::begin(base_), total, std::min(partitions_, total)};
	}

	constexpr std::default_sentinel_t end() const noexcept { return {}; }

	constexpr size_type size() const {
		const auto total = static_cast<size_type>(std::ranges::size(base_));
		return std::min(partitions_, total);
	}

private:
	R base_;
	std::size_t partitions_;
};

struct RoundRobinAdaptor {
	std::size_t partitions;
};

template <std::ranges::viewable_range R>
constexpr auto operator|(R&& range, RoundRobinAdaptor adaptor) {
	return RoundRobinView{std::views::all(std::forward<R>(range)), adaptor.partitions};
}

constexpr RoundRobinAdaptor RoundRobin(std::size_t partitions) noexcept {
	return RoundRobinAdaptor{partitions};
}

}  // namespace atl::view

// vim: set ts=4 sw=4 noexpandtab:

