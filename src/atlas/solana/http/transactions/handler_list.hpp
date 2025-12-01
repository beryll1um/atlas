#pragma once

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>

#include <userver/server/handlers/http_handler_base.hpp>

#include <atlas/solana/rocksdb_indexer_fwd.hpp>

namespace atl::solana::http::transactions {

class HandlerListComponent final : public userver::server::handlers::HttpHandlerBase {
public:
	static constexpr std::string_view kName = "http-handler-transactions-list-solana";

	HandlerListComponent(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

	void HandleStreamRequest(userver::server::http::HttpRequest&, userver::server::request::RequestContext&,
			userver::server::http::ResponseBodyStream&) const final;

private:
	RocksDbIndexerPtr rocksdb_indexer_;
};

}  // namespace atl::solana::http::transactions

// vim: set ts=4 sw=4 noexpandtab:

