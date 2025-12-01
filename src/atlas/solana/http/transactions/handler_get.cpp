#include "handler_get.hpp"

#include <userver/http/content_type.hpp>
#include <userver/logging/log.hpp>

#include <atlas/solana/rocksdb_indexer.hpp>

#include <nlohmann/json.hpp>

namespace server = userver::server;

namespace atl::solana::http::transactions {

HandlerGetComponent::HandlerGetComponent(const userver::components::ComponentConfig& config,
		const userver::components::ComponentContext& context) :
	HttpHandlerBase{config, context},
	rocksdb_indexer_{context.FindComponent<RocksDbIndexerComponent>(
		"rocksdb-indexer-solana"
	).GetIndexer()} {}

std::string HandlerGetComponent::HandleRequestThrow(const server::http::HttpRequest& request,
		server::request::RequestContext& /*context*/) const {
	auto& response = request.GetHttpResponse();
	response.SetStatus(server::http::HttpStatus::kOk);
	response.SetContentType(userver::http::content_type::kApplicationJson);
	// Get the transaction by its signature specified in the path argument: /{signature}.
	auto transaction = rocksdb_indexer_->GetTransactionBySignature(request.GetPathArg("signature"));
	if (!transaction) {
		return "null";
	}
	auto json = nlohmann::json::from_cbor(*transaction);
	// It's possible to shrink the response using a JSON pointer; by default, the full response is provided.
	if (auto select = request.GetArg("select"); !select.empty()) {
		json = json[nlohmann::json::json_pointer{select}];
	}
	// Set the HTTP status to 200 OK and return the transaction JSON.
	response.SetStatus(server::http::HttpStatus::kOk);
	return json.dump();
}

}  // namespace atl::solana::http::transactions

// vim: set ts=4 sw=4 noexpandtab:

