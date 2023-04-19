#include <sodium.h>
#include <solana.hpp>

namespace solana {
namespace rpc {
Connection::Connection(const std::string &rpc_url,
                       const std::string &commitment)
    : rpc_url_(std::move(rpc_url)), commitment_(std::move(commitment)) {
  auto sodium_result = sodium_init();
  f = curl_easy_init();
  headers = NULL;

  if (sodium_result < -1)
    throw std::runtime_error("Error initializing sodium: " +
                             std::to_string(sodium_result));
}

///
/// 1. Build requests
///
json Connection::getAccountInfoRequest(const std::string &account,
                                       const std::string &encoding,
                                       const size_t offset,
                                       const size_t length) {
  json params = {account};
  json options = {{"encoding", encoding}};
  if (offset && length) {
    json dataSlice = {"dataSlice", {{"offset", offset}, {"length", length}}};
    options.emplace(dataSlice);
  }
  params.emplace_back(options);

  return jsonRequest("getAccountInfo", params);
}
json Connection::getMultipleAccountsRequest(
    const std::vector<std::string> &accounts, const std::string &encoding,
    const size_t offset, const size_t length) {
  json pubKeys = json::array();
  for (auto &account : accounts) {
    pubKeys.emplace_back(account);
  }
  json params = {};
  params.emplace_back(pubKeys);
  json options = {{"encoding", encoding}};
  if (offset && length) {
    json dataSlice = {"dataSlice", {{"offset", offset}, {"length", length}}};
    options.emplace(dataSlice);
  }
  params.emplace_back(options);

  return jsonRequest("getMultipleAccounts", params);
}
json Connection::getBlockhashRequest(const std::string &commitment,
                                     const std::string &method) {
  const json params = {{{"commitment", commitment}}};

  return jsonRequest(method, params);
}

json Connection::sendTransactionRequest(
    const std::string &transaction, const std::string &encoding,
    bool skipPreflight, const std::string &preflightCommitment) {
  const json params = {transaction,
                       {{"encoding", encoding},
                        {"skipPreflight", skipPreflight},
                        {"preflightCommitment", preflightCommitment}}};

  return jsonRequest("sendTransaction", params);
}

///
/// 2. Invoke RPC endpoints
///
PublicKey Connection::getRecentBlockhash_DEPRECATED(
    const std::string &commitment) {
  const json req = getBlockhashRequest(commitment);
  std::stringstream ent_f;
  const std::string jsonSerialized = req.dump();

  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(f, CURLOPT_URL, rpc_url_.c_str());
  curl_easy_setopt(f, CURLOPT_COPYPOSTFIELDS, jsonSerialized.c_str());
  curl_easy_setopt(f, CURLOPT_POST, 1);
  curl_easy_setopt(f, CURLOPT_WRITEFUNCTION, CurlWrite);
  curl_easy_setopt(f, CURLOPT_WRITEDATA, &ent_f);
  curl_easy_setopt(f, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(f, CURLOPT_HTTPHEADER, headers);
  CURLcode b = curl_easy_perform(f);
  curl_slist_free_all(headers);
  if (b == CURLE_OK) ;
  else
    throw std::runtime_error("unexpected status_code ");
  json res = json::parse(ent_f.str().c_str());
  const std::string encoded = res["result"]["value"]["blockhash"];
  return PublicKey::fromBase58(encoded);
}
Blockhash Connection::getLatestBlockhash(const std::string &commitment) {
  const json req = getBlockhashRequest(commitment, "getLatestBlockhash");
  std::stringstream ent_f;
  const std::string jsonSerialized = req.dump();

  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(f, CURLOPT_URL, rpc_url_.c_str());
  curl_easy_setopt(f, CURLOPT_COPYPOSTFIELDS, jsonSerialized.c_str());
  curl_easy_setopt(f, CURLOPT_POST, 1);
  curl_easy_setopt(f, CURLOPT_WRITEFUNCTION, CurlWrite);
  curl_easy_setopt(f, CURLOPT_WRITEDATA, &ent_f);
  curl_easy_setopt(f, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(f, CURLOPT_HTTPHEADER, headers);
  CURLcode b = curl_easy_perform(f);
  curl_slist_free_all(headers);
  if (b == CURLE_OK) ;
  else
    throw std::runtime_error("unexpected status_code ");
  
  json res = json::parse(ent_f.str().c_str());

  const std::string encoded = res["result"]["value"]["blockhash"];
  const uint64_t lastValidBlockHeight =
      static_cast<uint64_t>(res["result"]["value"]["lastValidBlockHeight"]);
  return {PublicKey::fromBase58(encoded), lastValidBlockHeight};
}

uint64_t Connection::getBlockHeight(const std::string &commitment) {
  const json req = getBlockhashRequest(commitment, "getBlockHeight");
  std::stringstream ent_f;
  const std::string jsonSerialized = req.dump();

  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(f, CURLOPT_URL, rpc_url_.c_str());
  curl_easy_setopt(f, CURLOPT_COPYPOSTFIELDS, jsonSerialized.c_str());
  curl_easy_setopt(f, CURLOPT_POST, 1);
  curl_easy_setopt(f, CURLOPT_WRITEFUNCTION, CurlWrite);
  curl_easy_setopt(f, CURLOPT_WRITEDATA, &ent_f);
  curl_easy_setopt(f, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(f, CURLOPT_HTTPHEADER, headers);
  CURLcode b = curl_easy_perform(f);
  curl_slist_free_all(headers);
  if (b == CURLE_OK) ;
  else
    throw std::runtime_error("unexpected status_code ");   
  json res = json::parse(ent_f.str().c_str());

  const uint64_t blockHeight = res["result"];
  return blockHeight;
}
json Connection::getSignatureStatuses(
    const std::vector<std::string> &signatures, bool searchTransactionHistory) {
  const json params = {
      signatures, {{"searchTransactionHistory", searchTransactionHistory}}};

  return jsonRequest("getSignatureStatuses", params);
}

std::string Connection::signAndSendTransaction(
    const Keypair &keypair, const CompiledTransaction &tx, bool skipPreflight,
    const std::string &preflightCommitment) {
  std::vector<uint8_t> txBody;
  tx.serializeTo(txBody);

  const auto signedtx = keypair.privateKey.signMessage(txBody);
  const auto b58Sig =
      b58encode(std::string(signedtx.begin(), signedtx.end())).second;

  std::vector<uint8_t> signedTransaction;
  solana::CompactU16::encode(1, signedTransaction);
  signedTransaction.insert(signedTransaction.end(), signedtx.begin(), signedtx.end());
  signedTransaction.insert(signedTransaction.end(), txBody.begin(), txBody.end());

  const auto b64tx = b64encode(std::string(signedTransaction.begin(), signedTransaction.end()));
  const json req = sendTransactionRequest(b64tx, "base64", skipPreflight,
                                          preflightCommitment);
  const std::string jsonSerialized = req.dump();
  std::stringstream ent_f;

  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(f, CURLOPT_URL, rpc_url_.c_str());
  curl_easy_setopt(f, CURLOPT_COPYPOSTFIELDS, jsonSerialized.c_str());
  curl_easy_setopt(f, CURLOPT_POST, 1);
  curl_easy_setopt(f, CURLOPT_WRITEFUNCTION, CurlWrite);
  curl_easy_setopt(f, CURLOPT_WRITEDATA, &ent_f);
  curl_easy_setopt(f, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(f, CURLOPT_HTTPHEADER, headers);
  CURLcode b = curl_easy_perform(f);
  curl_slist_free_all(headers);
  if (b == CURLE_OK) ;
  else
    throw std::runtime_error("unexpected status_code ");

  json res = json::parse(ent_f.str().c_str());

  if (b58Sig == res["result"]) ;
  else
    throw std::runtime_error("could not submit tx: " + ent_f.str());

  return b58Sig;
}
}  // namespace rpc

}  // namespace solana