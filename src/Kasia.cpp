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

#include <Kasia.h>
#include <Arduino.h>
#include <WiFi.h>
// #include <esp_wifi_types.h>

extern "C"
{
    // #include <stdint.h>
    // #include <stdbool.h>
    // #include <stdio.h>
    // #include <stdlib.h>
    // #include <inttypes.h>
#include <string.h>

#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event.h>

#include <esp_http_server.h>
    // #include "lwip/ip_addr.h"
    // #include "lwip/opt.h"
    // #include "lwip/err.h"
    // #include "lwip/dns.h"
    // #include "dhcpserver/dhcpserver_options.h"

} // extern "C"
// #include "esp_wifi_types.h"

bool Kasia::_printToSerial = false;
String Kasia::_payload = "";

const char *Kasia::_deviceId = "Device Id was not set";
const char *Kasia::_lbl1 = NULL;
float *Kasia::_flt1 = NULL;
const char *Kasia::_lbl2 = NULL;
bool *Kasia::_bln1 = NULL;

// AsyncWebServer *Kasia::_server2 = NULL;

Kasia::THandlerFunction Kasia::test = []()
{
    Serial.println("static test");
};

Kasia::TActionBoolCharPtr Kasia::onGotIP = [](bool isNew, const char *ip)
{
    Kasia::println("Got ", isNew ? "new" : "same", " IP: ", ip);
};

// #define HTTP_PORT_TO_USE 80

// AsyncWebServer __server(HTTP_PORT_TO_USE);

Kasia::Kasia()
{
    // _server2 = &__server;
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::print(const char *text)
{
    if (_printToSerial)
        Serial.print(text);
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::println(const char *text)
{
    if (_printToSerial)
        Serial.println(text);
}

/**
 * print text line to serial
 * @param text
 */
template <class... TArgs>
void Kasia::println(TArgs &&...args)
{
    if (!_printToSerial)
        return;
    const char *texts[] = {args...};

    std::string line = "";
    for (auto &&t : texts)
    {
        line.append(t);
    }

    Serial.println(line.c_str());
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::bindData(const char *label, float *ptr)
{
    _lbl1 = label;
    _flt1 = ptr;

    String temp = label;
    temp += " : ";
    temp += String(*ptr);
    Serial.println(temp);
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::bindData(const char *label, bool *ptr)
{
    _lbl2 = label;
    _bln1 = ptr;
}

/**
 * print text line to serial
 * @param text
 */
bool Kasia::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::waitUntilConnected()
{
    while (true)
    {
        if (Kasia::isConnected())
            break;
        // TODO can't use delay
        delay(1);
    }
}

/**
 * print text line to serial
 * @param text
 */
bool Kasia::waitUntilConnected(unsigned int timeout)
{
    // get start time

    // get cuurrent time

    while (true)
    {
        if (Kasia::isConnected())
            return true;
        // TODO can't use delay
        delay(1);
    }

    return false;
}

// #define HTTP_PORT_TO_USE 80

AsyncWebServer server2(80);

void Kasia::startServer()
{

    HTTPClient http;

    http.begin("https://bromleysat.space/data/index.html");
    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK)
    {
        // Serial.print("HTTP Response code: ");
        // Serial.println(httpResponseCode);
        _payload = http.getString();
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();

    // auto server2 = __server;//*_server2;

    server2.on("/", HTTP_GET2, [](AsyncWebServerRequest *request)
               {
                   auto *response = request->beginResponse(200, "text/html", _payload);
                   //TODO these cache headers do not work and can try meta tags
                    //might need to return 304 when etag is the same
                    //might need to use if-none-match header
                   response->addHeader("cache-control", "public, s-maxage=36000, stale-while-revalidate=31536000");
                   request->send(response); });

    server2.on("/api/config", HTTP_GET2, [](AsyncWebServerRequest *request)
               {
      String json = "";
          json += "{";
          json += "\"version\":\""+String(KASIA_VERSION)+"\",";          
          json += "\"deviceId\":\""+String(_deviceId)+"\"";
          json += "}";

      request->send(200, "application/json", json);
      json = String(); });

    server2.on("/api/data", HTTP_GET2, [](AsyncWebServerRequest *request)
               {
      String json = "";
          json += "{";
          if(_lbl1 != NULL) 
          json += "\"" + String(_lbl1) + "\":"+ String(*_flt1);
           if(_lbl2 != NULL) {
                if(_lbl1 != NULL) json += ",";
                json += "\"" + String(_lbl2) + "\":"+ String((int)*_bln1);          
            }
          json += "}";

      request->send(200, "application/json", json);
      json = String(); });

    server2.begin();

    Kasia::print(_deviceId);
    Kasia::println(" server started!");
}

// #define ARDUHAL_LOG_LEVEL ARDUHAL_LOG_LEVEL_WARN

static void _arduino_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_EVENT_MAX;

    log_w("test warning");

    // int t = ARDUHAL_LOG_LEVEL;
    // Serial.println("got here");
    // Serial.println(t);

    /*
     * STA
     * */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        log_v("STA Started");
        // Serial.println("test");
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_START;
    }

    /*
     * STA
     * */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        log_e("STA Started");
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_START;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP)
    {
        log_e("STA Stopped");
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_STOP;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_AUTHMODE_CHANGE)
    {
        // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
        wifi_event_sta_authmode_change_t *event = (wifi_event_sta_authmode_change_t *)event_data;
        log_v("STA Auth Mode Changed: From: %s, To: %s", auth_mode_str(event->old_mode), auth_mode_str(event->new_mode));
        // #endif
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE;
        memcpy(&arduino_event.event_info.wifi_sta_authmode_change, event_data, sizeof(wifi_event_sta_authmode_change_t));
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
        wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
        // Serial.println("Connected");
        // Serial.println(WiFi.localIP());

        log_v("STA Connected: SSID: %s, BSSID: " MACSTR ", Channel: %u, Auth: %s", event->ssid, MAC2STR(event->bssid), event->channel, auth_mode_str(event->authmode));
        // #endif
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_CONNECTED;
        memcpy(&arduino_event.event_info.wifi_sta_connected, event_data, sizeof(wifi_event_sta_connected_t));
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
        log_e("STA Disconnected: SSID: %s, BSSID: " MACSTR ", Reason: %u", event->ssid, MAC2STR(event->bssid), event->reason);
        // #endif
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_DISCONNECTED;
        memcpy(&arduino_event.event_info.wifi_sta_disconnected, event_data, sizeof(wifi_event_sta_disconnected_t));
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        //  #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        // log_e("STA Got %sIP:" IPSTR, event->ip_changed ? "New " : "Same ", IP2STR(&event->ip_info.ip));
        //	#endif

        Kasia::onGotIP(event->ip_changed, IPAddress(IP2STR(&event->ip_info.ip)).toString().c_str());

        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_GOT_IP;
        memcpy(&arduino_event.event_info.got_ip, event_data, sizeof(ip_event_got_ip_t));
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        log_e("STA IP Lost");
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_LOST_IP;
    }
    else
    {
        log_e("Unknown event");
    }
}

/**
 * print text line to serial TODO
 * @param text
 */
void Kasia::setBaud(long baud)
{
    if (baud > -1)
    {
        _printToSerial = true;
        Serial.begin(baud);
    }
}

/**
 * print text line to serial TODO
 * @param text
 */
// void Kasia::onGotIP(void onMsg (bool isNew, const char *ip))
// {
//     //(&onMsg->)(true,"test");
// }
/**
 * print text line to serial TODO
 * @param text
 */
void Kasia::startWiFi()
{
    // if(isConnected()){
    //     return;
    // }

    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        log_e("esp_event_loop_create_default failed!");
        return; // err;
    }

    auto res = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_arduino_event_cb, NULL, NULL);

    log_e("   ||");

    int temp = res;

    // BromleySatIoT::printLn(temp);
    //  std::string s = std::to_string(temp);
    //  log_e(s);

    if (res == ESP_OK)
    {
        log_e("ESP_OK");
    }
    else if (res == ESP_FAIL)
    {
        log_e("ESP_FAIL");
    }
    else if (res == ESP_ERR_NO_MEM)
    {
        log_e("ESP_ERR_NO_MEM");
    }
    else if (res == ESP_ERR_INVALID_ARG)
    {
        log_e("ESP_ERR_INVALID_ARG");
    }
    else if (res == ESP_ERR_INVALID_STATE)
    {
        log_e("ESP_ERR_INVALID_STATE");
    }
    else
    {
        log_e("UNKNOWN");
    }

    log_e("||   ");

    if (res)
    {
        log_e("event_handler_instance_register for WIFI_EVENT Failed!");
        // return false;
    }

    if (esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &_arduino_event_cb, NULL, NULL))
    {
        log_e("event_handler_instance_register for IP_EVENT Failed!");
        //    return false;
    }

    //  start connection. This returns the result but we do not wait for it
    if (_ssid == NULL || _pwd == NULL)
    {
        // this will use WiFi creadentials that were saved at previous conection attempt
        WiFi.begin();
    }
    else
    {
        WiFi.begin(_ssid, _pwd);
    }

    // TODO refactor to non blocking
    waitUntilConnected();
    startServer();
}

/**
 * print text line to serial TODO
 * @param text
 */
void Kasia::start(const char *deviceId, long baud, const char *ssid, const char *pwd)
{
    _deviceId = deviceId;
    _ssid = ssid;
    _pwd = pwd;

    Kasia::setBaud(baud);
    Kasia::startWiFi();
}

/**
 * print text line to serial TODO
 * @param text
 */
void Kasia::start(const char *deviceId, long baud)
{
    _deviceId = deviceId;
    Kasia::setBaud(baud);
    Kasia::startWiFi();
}

/**
 * print text line to serial TODO
 * @param text
 */
void Kasia::start(const char *deviceId)
{
    _deviceId = deviceId;
    Kasia::startWiFi();
}

/**
 * print text line to serial TODO
 * @param text
 */
void Kasia::start()
{
    Kasia::startWiFi();
}

Kasia kasia;
