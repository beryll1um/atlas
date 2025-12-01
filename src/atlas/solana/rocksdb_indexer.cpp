#include "rocksdb_indexer.hpp"

#include <format>

#include <userver/kafka/consumer_component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include <userver/storages/rocks/db.hpp>
#include <userver/storages/rocks/component.hpp>
#include <userver/storages/rocks/write_batch.hpp>

#include <userver/logging/log.hpp>
#include <userver/utils/async.hpp>

#include <atlas/core/ranges.hpp>
#include <atlas/core/base58.hpp>

#include <nlohmann/json.hpp>

namespace utils = userver::utils;
namespace engine = userver::engine;

namespace {
struct Info {
	std::size_t slot{}, ord{};
};
[[maybe_unused]] void from_json(const nlohmann::json& json, Info& account) {
	json.at("slot").get_to(account.slot);
	json.at("ord").get_to(account.ord);
}
}  // namespace

namespace atl::solana {

TableId::TableId(std::uint64_t slot, std::uint64_t ord) : LexicographicKey{slot, ord} {}

AccountIndexId::AccountIndexId(std::string_view account, std::uint64_t slot, std::uint64_t ord) :
	LexicographicKey{slot, ord}
{
	auto account_chars = base58::FromString<char>(account);
	if (account_chars.size() != detail::kAccountSize) {
		throw std::runtime_error(std::format("Invalid decoded account size: {}", account));
	}
	this->stream_.write(account_chars);
}

}  // namespace atl::solana

namespace ColumnFamily {
constinit std::string kTable = "sol_tab";
constinit std::string kAccountIdx = "sol_acc2tab_idx";
constinit std::string kSignatureIdx = "sol_sig2tab_idx";
}  // namesapce ColumnFamily

namespace atl::solana {

RocksDbIndexer::RocksDbIndexer(rocks::DbPtr rocksdb) :
	rocksdb_{rocksdb},
	table_handle_{rocksdb_->GetColumnFamily(ColumnFamily::kTable)},
	account_index_handle_{rocksdb_->GetColumnFamily(ColumnFamily::kAccountIdx)},
	signature_index_handle_{rocksdb_->GetColumnFamily(ColumnFamily::kSignatureIdx)} {}

void RocksDbIndexer::Index(std::string_view signature, std::string_view json_str) {
	auto json = nlohmann::json::parse(json_str);
	// Write data into the table and separate index column families.
	rocks::WriteBatch batch{};
	auto info = json["info"].get<Info>();
	TableId table_id{info.slot, info.ord};
	// It make sense to encode everything to CBOR as it econom as a lot of data.
	auto cbor = nlohmann::json::to_cbor(json);
	batch.Put(table_handle_, table_id.AsStringView(),
		std::string_view{reinterpret_cast<const char*>(cbor.data()), cbor.size()});
	// This index is responsible for indexing transactions by account.
	for (auto&& account : json["payload"]["transaction"]["message"]["accountKeys"]) {
		AccountIndexId account_id{account.at("pubkey").get_ref<const std::string&>(), info.slot, info.ord};
		batch.Put(account_index_handle_, account_id.AsStringView(), table_id.AsStringView());
	}
	// The final one helps us to search transaction by signature.
	batch.Put(signature_index_handle_, signature, table_id.AsStringView());
	rocksdb_->Write(batch);
}

RocksDbIndexer::Iterator RocksDbIndexer::GetTableIter() {
	return rocksdb_->GetSnapshot().NewIterator(table_handle_);
}

RocksDbIndexer::Iterator RocksDbIndexer::GetAccountIndexIter() {
	return rocksdb_->GetSnapshot().NewIterator(account_index_handle_);
}

std::optional<std::string> RocksDbIndexer::GetTransaction(std::string_view table_id) {
	return rocksdb_->Get(table_handle_, table_id);
}

std::optional<std::string> RocksDbIndexer::GetTransactionBySignature(std::string_view signature) {
	auto table_id = rocksdb_->Get(signature_index_handle_, signature);
	if (!table_id) {
		return {};
	}
	return rocksdb_->Get(table_handle_, *table_id);
}

userver::yaml_config::Schema RocksDbIndexerComponent::GetStaticConfigSchema() {
	return userver::yaml_config::MergeSchemas<ComponentBase>(R"(
type: object
description: Solana RocksDB indexer component
additionalProperties: false
properties:
    kafka_consumer:
        type: string
        description: name of the associated Kafka consumer component
)");
}

RocksDbIndexerComponent::RocksDbIndexerComponent(const userver::components::ComponentConfig& config,
		const userver::components::ComponentContext& context) :
	ComponentBase{config, context},
	rocksdb_indexer_{std::make_shared<RocksDbIndexer>(context.FindComponent<userver::components::Rocks>().GetDb())},
	kafka_consumer_{
		context.FindComponent<kafka::ConsumerComponent>(config["kafka_consumer"].As<std::string>()).GetConsumer()
	},
	task_processor_{context.GetTaskProcessor("rocks-indexer-task-processor")}
{
	kafka_consumer_.Start([&](kafka::MessageBatchView batch) {
		std::vector<engine::TaskWithResult<void>> tasks;
		// Reserve asynchronous tasks storage to avoid reallocations.
		const auto worker_count = GetWorkerCount(task_processor_);
		tasks.reserve(worker_count);
		// Split incoming Kafka messages into chunks and process in parallel.
		for (auto&& chunk : batch | view::RoundRobin(worker_count)) {
			tasks.push_back(utils::Async(task_processor_, std::format("rocksdb_indexer[{}]", chunk.size()),
				[this, chunk] {
					for (auto&& message : chunk) {
						try {
							rocksdb_indexer_->Index(message.GetKey(), message.GetPayload());
						} catch (std::exception& exc) {
							LOG_ERROR() << "Failed to process signature: " << message.GetKey() << ": " << exc.what();
						}
					}
				}));
		}
		for (auto&& task : tasks) task.Wait();
		kafka_consumer_.AsyncCommit();
	});
}

RocksDbIndexerPtr RocksDbIndexerComponent::GetIndexer() {
	return rocksdb_indexer_;
}

}  // namespace atl::solana

// vim: set ts=4 sw=4 noexpandtab:

