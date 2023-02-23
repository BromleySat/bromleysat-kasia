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

#ifndef KASIA_H
#define KASIA_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <KasiaEncryption.h>
#include <KasiaCachedClient.h>
#include <KasiaLogger.h>

#define KASIA_VERSION "0.1.1"

#define KASIA_TYPE_INT32 "5"
#define KASIA_TYPE_FLOAT "7"
#define KASIA_TYPE_BOOL "9"

#ifdef KASIA_OVERRIDE_HTTP_PORT
#define KASIA_SERVER_HTTP_PORT KASIA_OVERRIDE_HTTP_PORT
#else
#define KASIA_SERVER_HTTP_PORT 80 // default http port
#endif

class Kasia : KasiaEncryption
{
public:
  typedef std::function<void(bool, const char *)> TActionBoolCharPtr;
  typedef std::function<std::string(void)> TFuncVoidString;
  Kasia();
  void bindData(const char *label, float *ptr);
  void bindData(const char *label, bool *ptr);
  void bindData(const char *label, int *ptr);
  void start(const char *deviceId, long baud, const char *ssid, const char *pwd);
  void start(const char *deviceId, long baud);
  void start(const char *deviceId);
  void start();
  bool isConnected();
  bool waitUntilConnected(ulong timeout);
  void waitUntilConnected();
  static TActionBoolCharPtr onGotIP;

protected:
  const char *_ssid;
  const char *_pwd;
  static const char *_deviceId;
  static std::string _dataConfig;
  static KasiaCachedClient *_client;
  bool _isFirstElement = true;
  bool _isStartCalled = false;

  static TaskHandle_t _kasiaOverwatchTaskHandle;

  void startOverwatch();
  void stopOverwatch();
  void startWiFi();
  void reconnect();
  void startServer();
  void bindData(const char *label, const char *type, Kasia::TFuncVoidString valueLambda);

  static void _arduino_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  static void overwatchTask(void *param);
};

extern Kasia kasia;

#endif /* KASIA_H */
