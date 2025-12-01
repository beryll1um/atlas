#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#include <atlas/core/endianness.hpp>

#include <gtest/gtest.h>

TEST(ToBigEndian, U64) {
	auto b8841 = atl::ToBigEndian<std::uint64_t>(1488);
	EXPECT_TRUE(b8841 == 14989386934772563968ULL);
}

#endif  // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

// vim: set ts=4 sw=4 noexpandtab:

