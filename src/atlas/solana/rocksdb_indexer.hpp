#pragma once

#include <atlas/solana/rocksdb_indexer_fwd.hpp>

#include <string>
#include <cstdint>
#include <optional>
#include <concepts>
#include <string_view>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>

#include <userver/kafka/consumer_scope.hpp>

#include <userver/storages/rocks/db_fwd.hpp>
#include <userver/storages/rocks/iterator.hpp>
#include <userver/storages/rocks/column_family.hpp>

#include <atlas/core/span.hpp>
#include <atlas/core/endianness.hpp>
#include <atlas/core/buffer_stream.hpp>

namespace rocks = userver::storages::rocks;
namespace kafka = userver::kafka;

namespace atl::solana {

namespace detail {

// Helps to construct a sequential identifier from a slot and an ordinal number
// to sort them lexicographically in time order.
template <std::size_t N>
class LexicographicKey {
public:
	using value_type = char;

	LexicographicKey(std::uint64_t slot, std::uint64_t ord) : stream_{buf_} {
		auto ord_be = ToBigEndian(ord);
		stream_.write(AsByteLike<char>(std::span{&ord_be, 1}));
		auto slot_be = ToBigEndian(slot);
		stream_.write(AsByteLike<char>(std::span{&slot_be, 1}));
	}

	[[nodiscard]] std::string_view AsStringView() const {
		return std::string_view{buf_.data(), buf_.size()};
	}

	auto operator<=>(std::string_view str) noexcept {
		return this->AsStringView() <=> str;
	}

protected:
	std::array<value_type, N> buf_;
	BufferStreamUnsafe<value_type, BufferStreamDirection::kBackward> stream_;
};

// The byte length of a Solana account encoded in Base58 is always constant.
inline constexpr std::size_t kAccountSize = 32;

}  // namespace detail

class TableId final : public detail::LexicographicKey<sizeof(std::uint64_t) * 2> {
public:
	TableId(std::uint64_t slot, std::uint64_t ord);
};

class AccountIndexId final : public detail::LexicographicKey<detail::kAccountSize + sizeof(std::uint64_t) * 2> {
public:
	AccountIndexId(std::string_view account, std::uint64_t slot, std::uint64_t ord);
};

template <typename T>
concept LexicographicKeyLike = requires(const T& key) {
	{ key.AsStringView() } -> std::convertible_to<std::string_view>;
};

class RocksDbIndexer final {
public:
	using Iterator = rocks::Iterator<rocks::IteratorDirection::kForward>;

	explicit RocksDbIndexer(rocks::DbPtr);

	void Index(std::string_view signature, std::string_view json_str);

	[[nodiscard]] Iterator GetTableIter();
	[[nodiscard]] Iterator GetAccountIndexIter();

	[[nodiscard]] std::optional<std::string> GetTransaction(std::string_view table_id);
	[[nodiscard]] std::optional<std::string> GetTransactionBySignature(std::string_view signature);

private:
	rocks::DbPtr rocksdb_;
	rocks::ColumnFamilyHandle table_handle_;
	rocks::ColumnFamilyHandle account_index_handle_;
	rocks::ColumnFamilyHandle signature_index_handle_;
};

class RocksDbIndexerComponent final :
	public userver::components::ComponentBase {
public:
	static constexpr std::string_view kName = "rocksdb-indexer-solana";
	static userver::yaml_config::Schema GetStaticConfigSchema();

	RocksDbIndexerComponent(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

	[[nodiscard]] RocksDbIndexerPtr GetIndexer();

private:
	RocksDbIndexerPtr rocksdb_indexer_;
	kafka::ConsumerScope kafka_consumer_;
	userver::engine::TaskProcessor& task_processor_;
};

}  // namespace atl::solana

// vim: set ts=4 sw=4 noexpandtab:

