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


#ifndef BROMLEYSAT_WIFI_H
#define BROMLEYSAT_WIFI_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

// template <typename ...T>
//     std::enable_if_t<(std::is_same_v<T, char>&&...)>

// template <class... TArgs>
//  std::enable_if_t<(std::is_same_v<T, char>&&...)>

#define KASIA_VERSION "0.0.2"

class Kasia
{
public:
  Kasia();
  void bindData(const char *label, float *ptr);
  void bindData(const char *label, bool *ptr);
  void start(const char *deviceId, long baud, const char *ssid, const char *pwd);
  void start(const char *deviceId, long baud);
  void start(const char *deviceId);
  void start();
  // TOOD either all prints are static or none are
  // might use a separate logger class global instance
  void print(const char *text);
  void println(const char *text);
  template <class... TArgs>
  static void println(TArgs &&...args);
  bool isConnected();
  void waitUntilConnected();
  bool waitUntilConnected(unsigned int timeout);

  typedef std::function<void(void)> THandlerFunction;
  typedef std::function<void(bool, const char *)> TActionBoolCharPtr;

  // TODO encapsulation
  static THandlerFunction test;
  static TActionBoolCharPtr onGotIP;

protected:
  const char *_ssid;
  const char *_pwd;

  static const char *_deviceId;
  static const char *_lbl1;
  static float *_flt1;

  static const char *_lbl2;
  static bool *_bln1;

  // TODO make port configurable
  // static AsyncWebServer *_server2;

  // TODO don't store
  static String _payload;

  // const unsigned long _baud;
  static bool _printToSerial;
  void setBaud(long baud);
  void startWiFi();
  void startServer();
  // void onGotIP(void onMsg (bool isNew, const char *ip));
};

extern Kasia kasia;

#endif /* BROMLEYSAT_WIFI_H */
