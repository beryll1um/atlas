#include <string_view>

#include <gtest/gtest.h>

#include <atlas/solana/rocksdb_indexer.hpp>

namespace {
// It's not the wisest solution, but at least it provides a friendly interface.
template <std::size_t N>
bool CompareTableId(atl::solana::TableId table_id, const char (&str)[N]) {
	auto view = table_id.AsStringView();
	return view.size() == N - 1 && std::equal(view.begin(), view.end(), reinterpret_cast<const char*>(str));
}
}  // namespace

TEST(RocksDbIndexer, TableId) {
	EXPECT_TRUE(CompareTableId({1,1}, "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01"));
	EXPECT_TRUE(CompareTableId({2,2}, "\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x02"));
	EXPECT_TRUE(CompareTableId({3,3}, "\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x03"));
}

// vim: set ts=4 sw=4 noexpandtab:

