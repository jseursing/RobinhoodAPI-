#include "HTTPPayload.h"


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Clear
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::Clear()
{
  Payload.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  ToString
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string HTTPPayload::ToString() const
{
  std::string output = "Header:\n";
  for (unsigned int i = 0; i < Header.size(); ++i)
  {
    output += Header[i].key + ": " + Header[i].val + "\n";
  }

  output += "Content: \n";
  for (unsigned int i = 0; i < Payload.size(); ++i)
  {
    output += Payload[i].key + ": " + Payload[i].val + "\n";
  }

  return output;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Add
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::Add(std::string key, 
                      std::string val, 
                      bool allowDuplicates, 
                      KeyVal::Tier tier)
{
  // Do not allow duplicates.
  if ((-1 == Contains(key)) ||
      (true == allowDuplicates))
  {
    KeyVal kv;
    kv.tier = tier;
    kv.key = key;
    kv.val = val;
    Payload.emplace_back(kv);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Add
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::Add(std::string key, 
                      unsigned int val, 
                      bool allowDuplicates,
                      KeyVal::Tier tier)
{
  // Do not allow duplicates.
  if ((-1 == Contains(key)) ||
      (true == allowDuplicates))
  {
    KeyVal kv;
    kv.tier = tier;
    kv.key = key;
    kv.val = std::to_string(val);
    Payload.emplace_back(kv);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddToHeader
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::AddToHeader(std::string key,
                              std::string val,
                              bool allowDuplicates)
{
  KeyVal kv;
  kv.key = key;
  kv.val = val;
  Header.emplace_back(kv);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Remove
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::Remove(std::string key)
{
  int index = Contains(key);
  if (-1 != index)
  {
    Payload.erase(Payload.begin() + index);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  RemoveFromHeader
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::RemoveFromHeader(std::string key)
{
  for (unsigned int i = 0; i < Header.size(); ++i)
  {
    if (0 == key.compare(Header[i].key.c_str()))
    {
      Header.erase(Header.begin() + i);
      break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Contains
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
int HTTPPayload::Contains(std::string key, KeyVal::Tier tier, unsigned int start) const
{
  int idx = -1;
  for (unsigned int i = start; i < Payload.size(); ++i)
  {
    if ((0 == Payload[i].key.compare(key)) &&
        (tier == Payload[i].tier))
    {
      idx = i;
      break;
    }
  }

  return idx;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Contains
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool HTTPPayload::KeyContainsValue(const char* key, const char* value) const
{
  int index = Contains(key);

  // Specified key not found
  if (-1 == index)
  {
    return false;
  }

  // String compare value to what is found in the payload.
  return (0 == Payload[index].val.compare(value));
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Get
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string HTTPPayload::Get(std::string key) const
{
  std::string value = "";

  int index = Contains(key);
  if (-1 != index)
  {
    value = Payload[index].val;
  }

  return value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Get
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string HTTPPayload::Get(std::string key, 
                             std::string subKey) const
{
  std::string value = "";

  int index = Contains(key);
  if (-1 != index)
  {
    index = Contains(subKey, KeyVal::SUBKEY, index);
    if (-1 != index)
    {
      value = Payload[index].val;
    }
  }

  return value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Get
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string HTTPPayload::Get(std::string key, 
                             std::string subKey, 
                             std::string subSubKey) const
{
  std::string value = "";

  int index = Contains(key);
  if (-1 != index)
  {
    index = Contains(subKey, KeyVal::SUBKEY, index);
    if (-1 != index)
    {
      index = Contains(subSubKey, KeyVal::SUB0_SUBKEY, index);
      if (-1 != index)
      {
        value = Payload[index].val;
      }
    }
  }

  return value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Get
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string HTTPPayload::Get(std::string key,
                             std::string subKey,
                             std::string subSubKey,
                             std::string subSubSubKey) const
{
  std::string value = "";

  int index = Contains(key);
  if (-1 != index)
  {
    index = Contains(subKey, KeyVal::SUBKEY, index);
    if (-1 != index)
    {
      index = Contains(subSubKey, KeyVal::SUB0_SUBKEY, index);
      if (-1 != index)
      {
        index = Contains(subSubSubKey, KeyVal::SUB1_SUBKEY, index);
        if (-1 != index)
        {
          value = Payload[index].val;
        }
      }
    }
  }

  return value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Update
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::Update(std::string key, std::string value)
{
  int index = Contains(key);
  if (-1 != index)
  {
    Payload[index].val = value;
  }
}

//
// LAZY
//
bool IsNumber(std::string str)
{
  bool numVal = true;
  for (std::string::const_iterator itr = str.begin(); itr != str.end(); ++itr)
  {
    if ((*itr == '.') ||
        (*itr == 0))
    {
      continue;
    }

    if ((*itr < 0x30) ||
        (*itr > 0x39))
    {
      numVal = false;
      break;
    }
  }
  
  return numVal;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildPOSTRequest
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::BuildPOSTRequest(std::string& req) const
{
  if (0 == Payload.size())
  {
    req = "";
    return;
  }

  // Check to see if application/json is specified, 
  // otherwise use urlencoded.
  bool isJson = false;
  for (size_t i = 0; i < Header.size(); ++i)
  {
    if (0 == Header[i].key.compare("Content-Type"))
    {
      if (std::string::npos != Header[i].val.find("json"))
      {
        isJson = true;
      }

      break;
    }
  }

  // URLENCODED
  if (false == isJson)
  {
    req = "";
    for (unsigned int i = 0; i < Payload.size() - 1; ++i)
    {
      req += Payload[i].key + "=" + Payload[i].val + '&';
    }

    req += Payload.back().key + "=" + Payload.back().val + '\0';
  }
  else
  {
    // JSON
    req = "{";
    for (unsigned int i = 0; i < Payload.size() - 1; ++i)
    {
      bool num_val = IsNumber(Payload[i].val);
        
      req += "\"" + Payload[i].key + "\":" +
              (true == num_val ? " " : " \"") + 
              Payload[i].val + 
              (true == num_val ? "," : "\",");
    }

    bool num_val = IsNumber(Payload.back().val);
    req += "\"" + Payload.back().key + "\":" +
            (true == num_val ? " " : " \"") +
            Payload.back().val +
            (true == num_val ? "}" : "\"}") + '\0';
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildGETRequest
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::BuildGETRequest(std::string& req) const
{
  // TODO: Can there be more than one payload entry?
  if (0 < Payload.size())
  {
    if (0 < Payload[0].val.length())
    {
      req += '?';
      
      for (unsigned int i = 0; i < Payload.size(); ++i)
      {
        req += Payload[i].key + "=" + Payload[i].val + '&';
      }

      req.back() = '\0';
    } 
    else
    {
      req += Payload[0].key;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  JSONToPayload
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int HTTPPayload::JsonifyPayload(std::string data)
{
  // Clear the payload vector
  Payload.clear();

  // Call the real Jsonify function..
  KeyVal::Tier tier = KeyVal::KEY;
  Jsonify(data, tier, '}');

  return Payload.size();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetRawData
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::SetRawData(std::string& data)
{
  RawData = data;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  JSONToPayload
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string& HTTPPayload::GetRawData()
{
  return RawData;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetHeader
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<HTTPPayload::KeyVal>& HTTPPayload::GetHeader()
{
  return Header;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetPayload
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<HTTPPayload::KeyVal>& HTTPPayload::GetPayload()
{
  return Payload;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Constructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
HTTPPayload::HTTPPayload()
{

}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Destructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
HTTPPayload::~HTTPPayload()
{

}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Jsonify
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int HTTPPayload::Jsonify(std::string data, KeyVal::Tier& tier, char endChar)
{
  // We expect data to follow the following formats:
  // {"key": "value", ...}
  // {"key": [{"key": "value", ...}], ...}
  // {"key": {"key": "value", ...}, ...}

  // Retrieve the end position
  unsigned int currPos = data.find("{");
  if (std::string::npos != currPos)
  {
    int endPos = data.length();
    while (currPos < endPos)
    {
      // If the next character is a closing brace/bracket, exit now.
      if ((endChar == data[currPos]) &&
          (KeyVal::KEY != tier))
      {
        ++currPos;
        break;
      }

      // Check for end of array and exit if found.
      if (('}' == data[currPos]) &&
          (']' == data[currPos + 1]))
      {
        currPos += 2;
        break;
      }

      // Retrieve the boundary of the current entry.
      // If there is no position for delimiter, we've reached the end.
      unsigned int delimiter = data.find(":", currPos + 1);
      if (std::string::npos == delimiter)
      {
        break;
      }

      // Extract the key 
      unsigned int entryEnd = data.find_first_of(",}", delimiter + 1);

      std::string key = data.substr(currPos + 1, delimiter - currPos - 1);
      StringRemove(key, "\""); // Replace apostrophes...
      StringRemove(key, " ");  // Replace spaces...
      StringRemove(key, ",");  // Remove commas
      StringRemove(key, "{");  // Remove bracket
      StringRemove(key, "]");  // Remove bracket

      // Set currPos to the next position not containing a space.
      currPos = data.find_first_not_of(' ', delimiter + 1);
      if (std::string::npos == currPos)
      {
        break; // Error
      }

      // This is an array of key-value pairs
      if ('[' == data[currPos])
      {
        // Shift currPos over by one to skip the bracket
        ++currPos;

        if (']' != data[currPos]) // Check for empty array
        {
          // Add an empty value to key
          Add(key, "", true, tier);

          // Recursively parse the rest of the data
          tier = static_cast<KeyVal::Tier>(tier + 1);
          currPos += Jsonify(data.substr(currPos), tier, ']');
          tier = static_cast<KeyVal::Tier>(tier - 1);
        }
        else
        {
          ++currPos;
        }
      }
      else if ('{' == data[currPos])
      {
        if ('}' != data[currPos + 1]) // Check for empty brace
        {
          // Add an empty value to key
          Add(key, "", true, tier);

          // Recursively parse the rest of the data
          tier = static_cast<KeyVal::Tier>(tier + 1);
          currPos += Jsonify(data.substr(currPos), tier, '}');
          tier = static_cast<KeyVal::Tier>(tier - 1);
        }
        else
        {
          currPos += 2;
        }
      }
      else
      {
        // If the currPos character is a value, add it to the payload.
        std::string val = data.substr(currPos, entryEnd - currPos);
        StringRemove(val, "\\"); // Replace forward slashes...
        StringRemove(val, "\""); // Replace apostrophes...
        StringRemove(val, " ");  // Replace spaces...

        // Add the new entry and shift currentPos
        Add(key, val, true, tier);
        currPos = entryEnd;
      }
    }

    return currPos;
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  StringRemove
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPPayload::StringRemove(std::string& str, const char* delim)
{
  unsigned int base = 0;
  unsigned int next = str.find_first_of(delim, base);
  while (std::string::npos != next)
  {
    str = str.substr(base, next) +
          str.substr(next + strlen(delim));
    base = 0;
    next = str.find_first_of(delim, base);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildRequestHeaders
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPRequest::BuildRequestHeaders(HINTERNET& httpRequest,
                                      unsigned int contentLen) const
{
  for (KeyVal kv : Header)
  {
    std::string mime = kv.key + ": " + kv.val;
    if (FALSE == HttpAddRequestHeadersA(httpRequest,
                                        mime.c_str(),
                                        mime.length(),
                                        HTTP_ADDREQ_FLAG_COALESCE))
    {
      printf("%s: Failed adding header '%s'\n", __FUNCTION__, mime.c_str());
    }
  }

  // Add content length if specified
  if (0 != contentLen)
  {
    std::string mime = "Content-Length: " + std::to_string(contentLen);
    if (FALSE == HttpAddRequestHeadersA(httpRequest,
                                        mime.c_str(),
                                        mime.length(),
                                        HTTP_ADDREQ_FLAG_COALESCE))
    {
      printf("%s: Failed adding Content-Length\n", __FUNCTION__);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildResponseHeaders
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPResponse::BuildResponseHeaders(std::string& data)
{
  // We expect the data to have the following format:
  // key: value
  // key: value ..

  // Retrieve the status "HTTP/?.? STATUS ?"
  unsigned int currentPos = data.find(" ");
  if (std::string::npos != currentPos)
  {
    ++currentPos;

    unsigned int endLinePos = data.find(" ", currentPos);
    if (std::string::npos != endLinePos)
    {
      std::string status = data.substr(currentPos, endLinePos);
      if (4 < status.length())
      {
        status[endLinePos - currentPos] = 0; // Null-Terminate
        if ((0x30 <= status[0]) &&
            (0x39 >= status[0]))
        {
          Status = atoi(status.c_str());

          // Retrieve the rest of the headers
          currentPos = data.find("\r\n") + 2;
          if (std::string::npos != currentPos)
          {
            endLinePos = data.find("\r\n", currentPos + 1);
            while (std::string::npos != endLinePos)
            {
              std::string currentLine(data.substr(currentPos, endLinePos - currentPos));
              if (2 < currentLine.length())
              {
                std::string key = currentLine.substr(0, currentLine.find(":"));
                if (2 < key.length())
                {
                  std::string val = currentLine.substr(key.length() + 2);
                  
                  KeyVal kv;
                  kv.key = key;
                  kv.val = val;
                  kv.tier = KeyVal::KEY;
                  Header.emplace_back(kv);
                }

                currentPos = endLinePos + 2;
                endLinePos = data.find("\r\n", currentPos + 1);
              }
            }
          }
        }
      }
    }
  }
}