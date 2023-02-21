/*
  Kasia WiFi and IoT library for internet enabled devices

  Copyright (c) 2023 BromleySat Ltd. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KASIA_CACHED_CLIENT_H
#define KASIA_CACHED_CLIENT_H

#include <WiFi.h>
#include <HTTPClient.h>

struct Header
{
  std::string Key;
  std::string Value;
  Header(std::string key, std::string value): Key(key), Value(value){}
};

class KasiaCachedClient
{
public:
  KasiaCachedClient(std::string baseUrl);
  void setHeader(std::string key, std::string value);
  std::string get();

protected:
  std::string _baseUrl;
  std::vector<Header> _headers;
  void removeHeader(std::string key);
};

#endif /* KASIA_CACHED_CLIENT_H */
