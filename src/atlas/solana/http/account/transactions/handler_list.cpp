#include "handler_list.hpp"

#include <atlas/solana/http_internal/handler_list.hpp>

namespace server = userver::server;

namespace atl::solana::http::account::transactions {

HandlerListComponent::HandlerListComponent(const userver::components::ComponentConfig& config,
		const userver::components::ComponentContext& context) :
	HttpHandlerBase{config, context},
	rocksdb_indexer_{context.FindComponent<RocksDbIndexerComponent>(
		"rocksdb-indexer-solana"
	).GetIndexer()} {}

void HandlerListComponent::HandleStreamRequest(server::http::HttpRequest& request, server::request::RequestContext&,
		server::http::ResponseBodyStream& stream) const {
	// Required path argument specifies which account to check `/{account}?` ...
	auto account = request.GetPathArg("account");
	if (account.empty()) {
		stream.SetStatusCode(server::http::HttpStatus::kBadRequest);
		return;
	}
	http_internal::HandleList<AccountIndexId>(request, stream, rocksdb_indexer_->GetAccountIndexIter(),
		[&](std::size_t slot, std::size_t ord) {
			return AccountIndexId{account, slot, ord};
		},
		[&](const RocksDbIndexer::Iterator& iter) {
			return rocksdb_indexer_->GetTransaction(iter->Value()).value();
		});
}

}  // namespace atl::solana::http::account::transactions

// vim: set ts=4 sw=4 noexpandtab:

