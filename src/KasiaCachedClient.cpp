#include "KasiaCachedClient.h"

KasiaCachedClient::KasiaCachedClient(std::string baseUrl)
{
  _baseUrl = baseUrl;
}

/**
 * Removes headers from the list of headers
 * If a header with the specified key does not exist, then the process still completes with success
 * @param key
 * Header key to be removed
 */
void KasiaCachedClient::removeHeader(std::string key)
{
  for (std::vector<Header>::iterator iter = _headers.begin(); iter != _headers.end(); ++iter)
  {
    if (iter->Key == key)
    {
      _headers.erase(iter);
      break;
    }
  }
}

/**
 * Set header to be used in the next http request
 * If a header with the same key already exists, it will be replaced
 * @param key
 * Header key/name eg. Authorization
 * @param value
 * Header text value eg. Basic <token>
 */
void KasiaCachedClient::setHeader(std::string key, std::string value)
{
  removeHeader(key);
  _headers.push_back(Header(key, value));
}

/**
 * Executes an HTTP_GET request and returns the value as string
 * @return
 * Returns response text as a string
 */
std::string KasiaCachedClient::get()
{
  HTTPClient http;
  http.begin(_baseUrl.c_str());

  for (auto &&h : _headers)
  {
    http.addHeader(h.Key.c_str(), h.Value.c_str());
  }

  int httpResponseCode = http.GET();
  std::string payload;

  if (httpResponseCode == HTTP_CODE_OK)
  {
    payload = http.getString().c_str();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    payload = "Error: Please wait a bit and try to reload the page";
  }
  http.end();
  return payload;
}
