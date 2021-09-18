#pragma once
#include "Asset.h"

class Portfolio
{
public:
  struct HistoricalEntry
  {
    std::string date;
    float open;
    float close;
  };

  RobinConstants::StatusType BuildPortfolioProfile();
  RobinConstants::StatusType LoadAccountProfile();
  RobinConstants::StatusType BuildHistoricalProfile
    (RobinConstants::Span span,
     RobinConstants::Interval interval = RobinConstants::DEFAULT_INTERVAL);
  std::string GetUserId() const;
  std::string GetEquity() const;
  std::string GetPrevClose() const;
  std::string GetBuyingPower() const;
  std::string GetCurrencyCode() const;
  std::string GetAccountNumber() const;
  std::string GetCryptoAccountId() const;
  std::vector<HistoricalEntry>* GetHistory();
  int GetAssetBySymbol(std::string symbol) const;

  void AddStock(std::string url, float quantity, float averagePrice);
  bool AddStock(std::string symbol);
  bool AddCrypto(std::string symbol, float quantity, float averagePrice);
  bool RemoveAsset(std::string symbol);
  std::vector<Asset*>& GetAssets();
  Portfolio();

private:

  // Members
  std::string UserId;
  std::string Equity;
  std::string PrevClose;
  std::string BuyingPower;
  std::string CurrencyCode;
  std::string AccountNumber;
  std::string CryptoAccountId;
  std::vector<HistoricalEntry> History;
  std::vector<Asset*> Assets;
};

