#pragma once

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>

#include <userver/server/handlers/http_handler_base.hpp>

#include <atlas/solana/rocksdb_indexer_fwd.hpp>

namespace atl::solana::http::transactions {

class HandlerGetComponent final : public userver::server::handlers::HttpHandlerBase {
public:
	static constexpr std::string_view kName = "http-handler-transactions-get-solana";

	HandlerGetComponent(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

	std::string HandleRequestThrow(const userver::server::http::HttpRequest&,
			userver::server::request::RequestContext&) const final;

private:
	RocksDbIndexerPtr rocksdb_indexer_;
};

}  // namespace atl::solana::http::transactions

// vim: set ts=4 sw=4 noexpandtab:

