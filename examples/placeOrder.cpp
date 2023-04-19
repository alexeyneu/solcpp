#include <spdlog/spdlog.h>

#include <chrono>
#include <future>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <curl/curl.h>
#include <condition_variable>
#include <atomic>
#include <iomanip>

#include "mango_v3.hpp"
#include "solana.hpp"

std::mutex com_mp;            // wait lock
std::condition_variable li;
std::atomic<bool> tro;

using json = nlohmann::json;

int main() 
{
  std::pair<std::pair<bool,bool>,bool> buy({}, !false);
 std::pair<std::pair<bool,bool>,bool> sell({}, false);
  const auto &config = mango_v3::MAINNET;
  int q;
	CURL *curl;
	CURL *f;

    curl = curl_easy_init();
    f = curl_easy_init();
  do {

    std::stringstream ent;


    curl_easy_setopt(curl, CURLOPT_URL, "http://185.251.91.6:5140/run");
     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
   curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_PORT, 5140);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite);    
         
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ent);
curl_easy_perform(curl);
	std::string ctr(ent.str()); 
    if(ctr== "buy") buy = buy.first.second == false ? std::pair(std::pair(!false, !false), buy.second) : std::pair(std::pair(false, !false), buy.second);
    if(ctr== "sell") sell = sell.first.second == false ? std::pair(std::pair(!false, !false), sell.second) : std::pair(std::pair(false, !false), sell.second);
    std::cout << ctr << '\r' << std::flush;

    if((sell.first.first == !false) && (sell.second == !false))
    {
	    sell.first.first= false;
      auto connection = solana::rpc::Connection(config.endpoint);
      auto recentBlockHash = connection.getLatestBlockhash();
      const auto group =
          connection.getAccountInfo<mango_v3::MangoGroup>(config.group);

      const auto symbolIt =
          std::find(config.symbols.begin(), config.symbols.end(), "SOL");
      const auto marketIndex = symbolIt - config.symbols.begin();
      assert(config.symbols[marketIndex] == "SOL");

      const auto perpMarketPk = group.perpMarkets[marketIndex].perpMarket;

      const auto market =
          connection.getAccountInfo<mango_v3::PerpMarket>(perpMarketPk.toBase58());
      assert(market.mangoGroup.toBase58() == config.group);

      const auto recentBlockhash = connection.getLatestBlockhash();
      const auto groupPk = solana::PublicKey::fromBase58(config.group);
      const auto programPk = solana::PublicKey::fromBase58(config.program);
      const auto keypair =
          solana::Keypair::fromFile("/home/alex/.ids/id.json");
      const auto mangoAccount = solana::PublicKey::fromBase58(
          "FKdedGrTeDLkjTEfwVycf4hFaqyU9GLA2Bs2g5YCvWkV");

      const mango_v3::ix::CancelAllPerpOrders cancelData = {
          mango_v3::ix::CancelAllPerpOrders::CODE, 4};


      const auto pqAsk = mango_v3::ix::uiToNativePriceQuantity(41, 17, config,
                                                               marketIndex, market);

      const mango_v3::ix::PlacePerpOrder placeAskData = {
          mango_v3::ix::PlacePerpOrder::CODE,
          pqAsk.first,
          pqAsk.second,
          1,
          mango_v3::Side::Sell,
          mango_v3::ix::OrderType::Market,
          false};

      const std::vector<solana::Instruction> ixs = {
          placePerpOrderInstruction(placeAskData, keypair.publicKey, mangoAccount,
                                    perpMarketPk, market, groupPk, group,
                                    programPk)};

      const auto tx = solana::CompiledTransaction::fromInstructions(
          ixs, keypair.publicKey, recentBlockhash);

      const auto b58Sig = connection.signAndSendTransaction(keypair, tx);
      spdlog::info(
          "placed order. check: https://explorer.solana.com/tx/{}?cluster=mainnet",
          b58Sig);
      const auto timeoutBlockHeight =
        recentBlockHash.lastValidBlockHeight +
        mango_v3::MAXIMUM_NUMBER_OF_BLOCKS_FOR_TRANSACTION;
      do
      {
        while (!false) {
            // Check if we are past validBlockHeight
            auto currentBlockHeight = connection.getBlockHeight("confirmed");
            if (timeoutBlockHeight <= currentBlockHeight) {
              spdlog::error("Timed out for txid: {}, current BlockHeight: {}", b58Sig,
                            currentBlockHeight);
              break;
            }
            const json req = connection.getSignatureStatuses({b58Sig});
            const std::string jsonSerialized = req.dump();
            spdlog::info("REQ: {}", jsonSerialized);

            std::stringstream ent_f;
    	       struct curl_slist *headers = NULL;
    	      headers = curl_slist_append(headers, "Expect:");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(f, CURLOPT_URL, config.endpoint.c_str());
            curl_easy_setopt(f, CURLOPT_COPYPOSTFIELDS, jsonSerialized.c_str());
            curl_easy_setopt(f, CURLOPT_POST, 1);
    		    curl_easy_setopt(f, CURLOPT_WRITEFUNCTION, CurlWrite);
            curl_easy_setopt(f, CURLOPT_WRITEDATA, &ent_f);
            curl_easy_setopt(f, CURLOPT_HTTPHEADER, headers);
    		    CURLcode b = curl_easy_perform(f);
    		    curl_slist_free_all(headers);
    		    if (b == CURLE_OK) {
    		      std::string e_f(ent_f.str());
                       spdlog::info("RES: {}", e_f.c_str());
            json res = json::parse(e_f.c_str());
              if (res["result"]["value"][0] != nullptr) { sell.first.second= false;sell.second= false;buy.second = !false; break; }
            } else {
             spdlog::error("Error: {}",   curl_easy_strerror(b));
             break;
           }
        }
      }while(sell.second== !false);
    }
    if((buy.first.first == !false) && (buy.second == !false))
    {
	    buy.first.first= false;
      auto connection = solana::rpc::Connection(config.endpoint);
      auto recentBlockHash = connection.getLatestBlockhash();
      const auto group =
          connection.getAccountInfo<mango_v3::MangoGroup>(config.group);

      const auto symbolIt =
          std::find(config.symbols.begin(), config.symbols.end(), "SOL");
      const auto marketIndex = symbolIt - config.symbols.begin();
      assert(config.symbols[marketIndex] == "SOL");

      const auto perpMarketPk = group.perpMarkets[marketIndex].perpMarket;

      const auto market =
          connection.getAccountInfo<mango_v3::PerpMarket>(perpMarketPk.toBase58());
      assert(market.mangoGroup.toBase58() == config.group);

      const auto recentBlockhash = connection.getLatestBlockhash();
      const auto groupPk = solana::PublicKey::fromBase58(config.group);
      const auto programPk = solana::PublicKey::fromBase58(config.program);
      const auto keypair =
          solana::Keypair::fromFile("/home/alex/.ids/id.json");
      const auto mangoAccount = solana::PublicKey::fromBase58(
          "FKdedGrTeDLkjTEfwVycf4hFaqyU9GLA2Bs2g5YCvWkV");

      const mango_v3::ix::CancelAllPerpOrders cancelData = {
          mango_v3::ix::CancelAllPerpOrders::CODE, 4};

      const auto pqBid = mango_v3::ix::uiToNativePriceQuantity(42, 17, config,
                                                               marketIndex, market);

      const mango_v3::ix::PlacePerpOrder placeBidData = {
          mango_v3::ix::PlacePerpOrder::CODE,
          pqBid.first,
          pqBid.second,
          1,
          mango_v3::Side::Buy,
          mango_v3::ix::OrderType::Market,
          false};


      const std::vector<solana::Instruction> ixs = {
          placePerpOrderInstruction(placeBidData, keypair.publicKey, mangoAccount,
                                    perpMarketPk, market, groupPk, group,
                                    programPk)};

      const auto tx = solana::CompiledTransaction::fromInstructions(
          ixs, keypair.publicKey, recentBlockhash);

      const auto b58Sig = connection.signAndSendTransaction(keypair, tx);
      spdlog::info(
          "placed order. check: https://explorer.solana.com/tx/{}?cluster=mainnet",
          b58Sig);
      const auto timeoutBlockHeight =
        recentBlockHash.lastValidBlockHeight +
        mango_v3::MAXIMUM_NUMBER_OF_BLOCKS_FOR_TRANSACTION;
      do{
      while (!false) {
        // Check if we are past validBlockHeight
        auto currentBlockHeight = connection.getBlockHeight("confirmed");
        if (timeoutBlockHeight <= currentBlockHeight) {
          spdlog::error("Timed out for txid: {}, current BlockHeight: {}", b58Sig,
                        currentBlockHeight);
          break;
        }
        const json req = connection.getSignatureStatuses({b58Sig});
        const std::string jsonSerialized = req.dump();
        spdlog::info("REQ: {}", jsonSerialized);
	std::stringstream ent_f;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl_easy_setopt(f, CURLOPT_URL, config.endpoint.c_str());
	curl_easy_setopt(f, CURLOPT_COPYPOSTFIELDS, jsonSerialized.c_str());
	curl_easy_setopt(f, CURLOPT_POST, 1);
	curl_easy_setopt(f, CURLOPT_WRITEFUNCTION, CurlWrite);
  curl_easy_setopt(f, CURLOPT_WRITEDATA, &ent_f);
	curl_easy_setopt(f, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(f, CURLOPT_HTTPHEADER, headers);
	CURLcode b = curl_easy_perform(f);

	curl_slist_free_all(headers);
  if (b == CURLE_OK) {
    std::string e_f(ent_f.str());
		spdlog::info("RES: {}", e_f.c_str());
		json res = json::parse(e_f.c_str());
	        if (res["result"]["value"][0] == nullptr) ;
          else { sell.first.second = false; buy.second = false; sell.second = !false; break; }
    } else {
		spdlog::error("Error: {}",   curl_easy_strerror(b));
		break;
	}

      }
      }while(buy.second == !false);
    }
    std::unique_lock<std::mutex> lk(com_mp);
    li.wait_for(lk , std::chrono::milliseconds(17000) , [] { return tro == !false; });
  } while(tro == false);
}
