# Atlas
**Atlas** is a plain-JSON transaction registry designed to unlock
**full historical visibility** and **high-throughput access** to Solana
blockchain data beyond the limits of standard RPC interfaces.

## Why Atlas?
The default Solana RPC interface is not built for data-intensive workloads:

- ❌ No efficient batch retrieval per account  
- ❌ No server-side filtering  
- ❌ Limited historical traversal  
- ❌ High network overhead for analytics use cases  

**Atlas** solves these problems by implementing an independent storage.

By maintaining a complete, indexed transaction history and exposing it via
a clean HTTP API, **Atlas** enables **analytics, compliance, explorers,
indexers, and trading infrastructure** to operate with speed and precision.

## Architecture Highlights
- **Complete historical storage** of Solana transactions  
- **Timeline-optimized indexing** for fast forward/backward navigation  
- **Plain JSON output** — zero decoding friction  
- **Deterministic performance**, bounded only by RocksDB throughput and
  underlying hardware capacity  

## API Endpoints
| Method | Endpoint                                 | Description                             |
|--------|------------------------------------------|-----------------------------------------|
| GET    | `/solana/transactions`                   | List transactions across the registry   |
| GET    | `/solana/transaction/{signature}`        | Fetch a transaction by signature        |
| GET    | `/solana/account/{account}/transactions` | Transactions mentioning a given account |

## Query Parameters
### Range Navigation
Efficiently traverse the blockchain timeline with slot-level precision.

| Parameter     | Description                             |
|---------------|-----------------------------------------|
| `start.slot`  | Slot to start from                      |
| `start.ord`   | Transaction index within the start slot |
| `limit.slot`  | Slot to stop at                         |
| `limit.ord`   | Last transaction index in the end slot  |

### Sorting
- `sort=desc` — newest to oldest (**default**)  
- `sort=asc`  — oldest to newest  

### Output Control
| Parameter | Description                                   |
|-----------|-----------------------------------------------|
| `count`   | Maximum number of transactions returned       |
| `select`  | JSON Pointer query to extract specific fields |

**Examples of `select`:**
- `/payload/transaction/signatures/0`  
- `/payload/transaction/message/accountKeys`  
- `/payload/meta/fee`  

This enables **low-bandwidth, high-signal queries**—ideal
for analytics pipelines.

## Example Request
Retrieve the last 10 transaction signatures involving a specific account:
```bash
curl -v --get \
  --data-urlencode "select=/payload/transaction/signatures/0" \
  "http://localhost:80/solana/account/BwWK17cbHxwWBKZkUYvzxLcNQ1YVyaFezduWbtm2de6s/transactions?count=10"

