#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/component.hpp>
#include <userver/utils/daemon_run.hpp>

#include <userver/kafka/consumer_component.hpp>

#include <userver/storages/rocks/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>

#include <atlas/solana/rocksdb_indexer.hpp>
#include <atlas/solana/http/transactions/handler_get.hpp>
#include <atlas/solana/http/transactions/handler_list.hpp>
#include <atlas/solana/http/account/transactions/handler_list.hpp>

using namespace atl::solana;

int main(int argc, char* argv[]) {
	auto components_list = userver::components::MinimalServerComponentList()
		.Append<userver::components::Rocks>()
		// I don't know why it's imposible to derive from this class...
		.Append<userver::kafka::ConsumerComponent>("kafka-consumer-solana")
		// Another stupid (IMHO) decision to explicitly make Kafka dependent
		// on Secdist.
		.Append<userver::components::Secdist>()
		.Append<userver::components::DefaultSecdistProvider>()
		// Uses Kafka consumer component to collect transactions
		// and store them in RocksDB.
		.Append<RocksDbIndexerComponent>()
		.Append<http::transactions::HandlerGetComponent>()
		.Append<http::transactions::HandlerListComponent>()
		.Append<http::account::transactions::HandlerListComponent>();
	return userver::utils::DaemonMain(argc, argv, components_list);
}

// vim: set ts=4 sw=4 noexpandtab:

