#include <spdlog/spdlog.h>
#include <curl/curl.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "mango_v3.hpp"

int main() {
  std::string rpc_url =
      "https://mango.rpcpool.com/946ef7337da3f5b8d3e4a34e7f88";
  auto connection = solana::rpc::Connection(rpc_url);
  const json req = connection.getAccountInfoRequest(
      "98pjRuQjK3qA6gXts96PqZT4Ze5QmnCmt3QYjhbUSPue");
  const std::string jsonSerialized = req.dump();
  spdlog::info("REQ: {}", jsonSerialized);
  CURL *f= curl_easy_init();
  std::stringstream ent_f;
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl_easy_setopt(f, CURLOPT_URL, rpc_url.c_str());
  curl_easy_setopt(f, CURLOPT_COPYPOSTFIELDS, jsonSerialized.c_str());
  curl_easy_setopt(f, CURLOPT_POST, 1);
  curl_easy_setopt(f, CURLOPT_WRITEFUNCTION, CurlWrite);
  curl_easy_setopt(f, CURLOPT_WRITEDATA, &ent_f);
  curl_easy_setopt(f, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(f, CURLOPT_HTTPHEADER, headers);
  CURLcode b = curl_easy_perform(f);

  curl_slist_free_all(headers);


  if ((b == CURLE_OK) == false) {
    spdlog::error("Error");
    return 1;
  } else {

    std::string e_f(ent_f.str());
    spdlog::info("RES: {}", e_f.c_str());
    json res = json::parse(e_f.c_str());

    const std::string encoded = res["result"]["value"]["data"][0];
    const std::string decoded = solana::b64decode(encoded);
    const mango_v3::MangoGroup *group =
        reinterpret_cast<const mango_v3::MangoGroup *>(decoded.data());
    spdlog::info("DEC:");
    spdlog::info("numOracles: {}", group->numOracles);

    for (int i = 0; i < mango_v3::MAX_TOKENS; ++i) {
      if (i >= group->numOracles && i != mango_v3::QUOTE_INDEX) continue;

      const auto &token = group->tokens[i];
      const auto mintPk = token.mint.toBase58();

      if (mintPk == std::string("11111111111111111111111111111111")) continue;

      const auto rootBankPk = token.rootBank.toBase58();
      if (i != mango_v3::QUOTE_INDEX) {
        spdlog::info("TOK: {}", i);
      } else {
        spdlog::info("QUOTE: {}", i);
      }
      spdlog::info("  mint: {}", mintPk);
      spdlog::info("  rootBank: {}", rootBankPk);
      spdlog::info("  decimals: {}", static_cast<int>(token.decimals));
    }

    for (int i = 0; i < mango_v3::MAX_PAIRS; ++i) {
      const auto &market = group->spotMarkets[i];
      const auto marketPk = market.spotMarket.toBase58();

      if (marketPk == std::string("11111111111111111111111111111111")) continue;

      spdlog::info("SPOT: {}", i);
      spdlog::info("  market: {}", marketPk);
      spdlog::info("  maintAssetWeight: {}",
                   market.maintAssetWeight.to_double());
      spdlog::info("  initAssetWeight: {}", market.initAssetWeight.to_double());
      spdlog::info("  maintLiabWeight: {}", market.maintLiabWeight.to_double());
      spdlog::info("  initLiabWeight: {}", market.initLiabWeight.to_double());
      spdlog::info("  liquidationFee: {}", market.liquidationFee.to_double());
    }

    for (int i = 0; i < mango_v3::MAX_PAIRS; ++i) {
      const auto &market = group->perpMarkets[i];
      const auto marketPk = market.perpMarket.toBase58();

      if (marketPk == std::string("11111111111111111111111111111111")) continue;

      spdlog::info("PERP: {}", i);
      spdlog::info("  market: {}", marketPk);
      spdlog::info("  maintAssetWeight: {}",
                   market.maintAssetWeight.to_double());
      spdlog::info("  initAssetWeight: {}", market.initAssetWeight.to_double());
      spdlog::info("  maintLiabWeight: {}", market.maintLiabWeight.to_double());
      spdlog::info("  initLiabWeight: {}", market.initLiabWeight.to_double());
      spdlog::info("  liquidationFee: {}", market.liquidationFee.to_double());
      spdlog::info("  makerFee: {}", market.makerFee.to_double());
      spdlog::info("  takerFee: {}", market.takerFee.to_double());
      spdlog::info("  baseLotSize: {}", market.baseLotSize);
      spdlog::info("  quoteLotSize: {}", market.quoteLotSize);
    }

    for (int i = 0; i < mango_v3::MAX_PAIRS; ++i) {
      const auto oraclePk = group->oracles[i].toBase58();

      if (oraclePk == std::string("11111111111111111111111111111111")) continue;

      spdlog::info("ORACLE {}: {}", i, oraclePk);
    }

    spdlog::info("signerNonce: {}", group->signerNonce);
    spdlog::info("signerKey: {}", group->signerKey.toBase58());
    spdlog::info("admin: {}", group->admin.toBase58());
    spdlog::info("dexProgramId: {}", group->dexProgramId.toBase58());
    spdlog::info("mangoCache: {}", group->mangoCache.toBase58());
    spdlog::info("validInterval: {}", group->validInterval);
    spdlog::info("insuranceVault: {}", group->insuranceVault.toBase58());
    spdlog::info("srmVault: {}", group->srmVault.toBase58());
    spdlog::info("msrmVault: {}", group->msrmVault.toBase58());
    spdlog::info("feesVault: {}", group->feesVault.toBase58());
    spdlog::info("maxMangoAccounts: {}", group->maxMangoAccounts);
    spdlog::info("numMangoAccounts: {}", group->numMangoAccounts);
  }
  return 0;
}
