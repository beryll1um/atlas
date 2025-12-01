#include "handler_list.hpp"

#include <atlas/solana/http_internal/handler_list.hpp>

namespace server = userver::server;

namespace atl::solana::http::transactions {

HandlerListComponent::HandlerListComponent(const userver::components::ComponentConfig& config,
		const userver::components::ComponentContext& context) :
	HttpHandlerBase{config, context},
	rocksdb_indexer_{context.FindComponent<RocksDbIndexerComponent>(
		"rocksdb-indexer-solana"
	).GetIndexer()} {}

void HandlerListComponent::HandleStreamRequest(server::http::HttpRequest& request, server::request::RequestContext&,
		server::http::ResponseBodyStream& stream) const {
	http_internal::HandleList<TableId>(request, stream, rocksdb_indexer_->GetTableIter(),
		[](std::size_t slot, std::size_t ord) {
			return TableId{slot, ord};
		},
		[&](const RocksDbIndexer::Iterator& iter) {
			return iter->Value();
		});
}

}  // namespace atl::solana::http::transactions

// vim: set ts=4 sw=4 noexpandtab:

