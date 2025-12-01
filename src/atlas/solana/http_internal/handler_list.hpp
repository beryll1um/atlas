#pragma once

#include <type_traits>

#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response_body_stream.hpp>

#include <userver/utils/from_string.hpp>
#include <userver/http/content_type.hpp>

#include <atlas/solana/rocksdb_indexer.hpp>

#include <nlohmann/json.hpp>

namespace atl::solana::http_internal {

template <class F, class R>
concept KeyFactoryLike = std::is_invocable_r_v<R, F, std::size_t, std::size_t>;

template <class F>
concept CborFetcherLike = std::is_invocable_r_v<std::string, F, const RocksDbIndexer::Iterator&>;

template <LexicographicKeyLike T>
void HandleList(userver::server::http::HttpRequest& request, userver::server::http::ResponseBodyStream& stream,
		RocksDbIndexer::Iterator iter, KeyFactoryLike<T> auto&& MakeKey, CborFetcherLike auto&& FetchCbor) {
	// Helps define the sort order as a unique state that is better localized with a function.
	enum class SortOrder : std::uint8_t { kAscending, kDescending };
	// Just shortcuts as they are used in several places below.
	constexpr auto kUllMax = std::numeric_limits<std::size_t>::max();
	constexpr auto kUllMin = std::numeric_limits<std::size_t>::min();

	std::size_t count;
	if (auto str = request.GetArg("count"); !str.empty()) {
		count = userver::utils::FromString<decltype(count)>(str);
	} else {
		stream.SetStatusCode(userver::server::http::HttpStatus::kBadRequest);
		return;
	}

	SortOrder sort = SortOrder::kDescending;
	if (auto str = request.GetArg("sort"); !str.empty()) {
		if (str == "asc") {
			sort = SortOrder::kAscending;
		} else if (str != "desc") {
			stream.SetStatusCode(userver::server::http::HttpStatus::kBadRequest);
			return;
		}
	}

	auto start_slot = sort == SortOrder::kAscending ? kUllMin : kUllMax;
	if (auto str = request.GetArg("start.slot"); !str.empty()) {
		start_slot = userver::utils::FromString<decltype(start_slot)>(str);
	}
	auto start_ord = sort == SortOrder::kAscending ? kUllMin : kUllMax;
	if (auto str = request.GetArg("start.ord"); !str.empty()) {
		start_ord = userver::utils::FromString<decltype(start_ord)>(str);
	}

	auto limit_slot = sort == SortOrder::kAscending ? kUllMax : kUllMin;
	if (auto str = request.GetArg("limit.slot"); !str.empty()) {
		limit_slot = userver::utils::FromString<decltype(limit_slot)>(str);
	}
	auto limit_ord = sort == SortOrder::kAscending ? kUllMax : kUllMin;
	if (auto str = request.GetArg("limit.ord"); !str.empty()) {
		limit_ord = userver::utils::FromString<decltype(limit_ord)>(str);
	}

	// It's possible to shrink the response using a JSON pointer; by default, the full response is provided.
	auto select = request.GetArg("select");
	auto json_ptr = select.empty() ? std::nullopt : std::optional{nlohmann::json::json_pointer{select}};

	auto start_key = MakeKey(start_slot, start_ord);
	if (sort == SortOrder::kAscending) {
		iter.Seek(start_key.AsStringView());
	} else {
		iter.SeekForPrev(start_key.AsStringView());
	}

	stream.SetHeader(std::string{"Content-Type"}, "application/json; charset=utf-8");

	auto limit_key = MakeKey(limit_slot, limit_ord);
	if (!count-- || !iter.Valid() ||
			(sort == SortOrder::kAscending
				? limit_key < iter->Key()
				: limit_key > iter->Key())) {
		stream.SetStatusCode(userver::server::http::HttpStatus::kNoContent);
		return;
	}

	stream.SetStatusCode(userver::server::http::HttpStatus::kOk);
	stream.SetEndOfHeaders();

	auto IterationStep = [&]{
		try {
			auto json = nlohmann::json::from_cbor(FetchCbor(iter));
			stream.PushBodyChunk(json_ptr ? json[*json_ptr].dump() : json.dump(), {});
		} catch (std::exception&) {
			stream.PushBodyChunk("null", {});
		}
		if (sort == SortOrder::kAscending) {
			++iter;
		} else {
			--iter;
		}
	};
	stream.PushBodyChunk("[", {});
	IterationStep();
	while (count-- && iter.Valid() &&
			(sort == SortOrder::kAscending
				? limit_key >= iter->Key()
				: limit_key <= iter->Key())) {
		stream.PushBodyChunk(",", {});
		IterationStep();
	}
	stream.PushBodyChunk("]", {});
}

}  // namespace atl::solana::http_internal

// vim: set ts=4 sw=4 noexpandtab:

