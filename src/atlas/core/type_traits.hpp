#pragma once

#include <cstddef>
#include <type_traits>

namespace atl {

template <class T>
struct IsByteLikeType : std::false_type {};

// The C++23 (N4950) paragraph 6.8.1 states:
// For any object (other than a potentially-overlapping subobject) of trivially copyable type T, whether or not
// the object holds a valid value of type T, the underlying bytes making up the object can be copied into
// an array of char, unsigned char, or std::byte.
template <> struct IsByteLikeType<char> : std::true_type {};
template <> struct IsByteLikeType<std::byte> : std::true_type {};
template <> struct IsByteLikeType<unsigned char> : std::true_type {};

template <class T>
inline constexpr bool kIsByteLikeType = IsByteLikeType<T>::value;

template <class T>
concept ByteLike = kIsByteLikeType<T>;

}  // namespace atl

// vim: set ts=4 sw=4 noexpandtab:

