#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <WinInet.h>

// Main HTTPPayload class
class HTTPPayload
{
public:
  // K-V Structure
  struct KeyVal
  {
    enum Tier
    {
      KEY,
      SUBKEY,
      SUB0_SUBKEY,
      SUB1_SUBKEY,
      SUB2_SUBKEY,
      SUB3_SUBKEY,
      SUB4_SUBKEY,
      SUB5_SUBKEY,
      SUB6_SUBKEY,
      SUB7_SUBKEY,
      SUB8_SUBKEY,
      SUB9_SUBKEY
    };

    Tier tier;
    std::string key;
    std::string val;
  };

  // Functions
  void Clear();
  std::string ToString() const;
  void Add(std::string key, 
           std::string val, 
           bool allowDuplicates = false, 
           KeyVal::Tier tier = KeyVal::KEY);
  void Add(std::string key, 
           unsigned int val, 
           bool allowDuplicates = false,
           KeyVal::Tier tier = KeyVal::KEY);
  void AddToHeader(std::string key,
                   std::string val,
                   bool allowDuplicates = false);
  void Remove(std::string key);
  void RemoveFromHeader(std::string key);
  int Contains(std::string key,  
               KeyVal::Tier tier = KeyVal::KEY,
               unsigned int start = 0) const;
  bool KeyContainsValue(const char* key, const char* value) const;
  std::string Get(std::string key) const;
  std::string Get(std::string key, 
                  std::string subKey) const;
  std::string Get(std::string key, 
                  std::string subKey, 
                  std::string subSubKey) const;
  std::string Get(std::string key, 
                  std::string subKey, 
                  std::string subSubKey, 
                  std::string subSubSubKey) const;
  void Update(std::string key, std::string value);
  void BuildPOSTRequest(std::string& req) const;
  void BuildGETRequest(std::string& req) const;
  unsigned int JsonifyPayload(std::string data);
  void SetRawData(std::string& data);
  std::string& GetRawData();
  std::vector<KeyVal>& GetHeader();
  std::vector<KeyVal>& GetPayload();
  HTTPPayload();
  ~HTTPPayload();

private:
  void StringRemove(std::string& str, const char* delim);
  unsigned int Jsonify(std::string data, KeyVal::Tier& tier, char endChar);

protected:
  // Members
  std::vector<KeyVal> Header;
  std::vector<KeyVal> Payload;
  std::string RawData;
};

// Derived children for neatness...
class HTTPRequest : public HTTPPayload
{
public:
  void BuildRequestHeaders(HINTERNET& httpRequest, unsigned int contentLen) const;
  HTTPRequest() : HTTPPayload() {};
  ~HTTPRequest() {};
};

class HTTPResponse : public HTTPPayload
{
public:
  unsigned int Status;
  std::string ErrorStatus;
  void BuildResponseHeaders(std::string& data);
  HTTPResponse() : HTTPPayload() {};
  ~HTTPResponse() {};
};