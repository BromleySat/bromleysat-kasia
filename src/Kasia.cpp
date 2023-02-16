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

extern "C"
{
#include <string.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event.h>

#include <esp_http_server.h>
#include "esp_task_wdt.h"
} // extern "C"

// TODO better encapsulation
std::vector<Kasia::TFuncVoidString> dataVectors;

std::string Kasia::_dataConfig = "";
const char *Kasia::_deviceId = "Device Id was not set";

Kasia::TActionBoolCharPtr Kasia::onGotIP = [](bool isNew, const char *ip)
{
    if (!kasiaLog.IsPrintingToSerial())
        return;
    Serial.print("Got ");
    Serial.print(isNew ? "new" : "same");
    Serial.print("IP: ");
    Serial.println(ip);
};

Kasia::Kasia()
{
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::bindData(const char *label, float *ptr)
{
    // TODO remove code duplication
    if (!_isFirstElement)
        _dataConfig.append("|");
    else
        _isFirstElement = false;

    _dataConfig.append("7"); // float
    _dataConfig.append(encode(label));

    dataVectors.push_back([=]()
                          { return std::to_string(*ptr); });
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::bindData(const char *label, bool *ptr)
{
    // TODO remove code duplication
    if (!_isFirstElement)
        _dataConfig.append("|");
    else
        _isFirstElement = false;

    _dataConfig.append("9"); // bool
    _dataConfig.append(encode(label));

    dataVectors.push_back([=]()
                          { return std::to_string((int16_t)*ptr); });
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::bindData(const char *label, int *ptr)
{
    // TODO remove code duplication
    if (!_isFirstElement)
        _dataConfig.append("|");
    else
        _isFirstElement = false;

    _dataConfig.append("5"); // int32
    _dataConfig.append(encode(label));

    dataVectors.push_back([=]()
                          { return std::to_string(*ptr); });
}

/**
 * print text line to serial
 * @param text
 */
void Kasia::bindAction(const char *label, TAction action)
{
    // actions.emplace(label, action);
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
        delay(1);
    }

    return false;
}

#ifdef KASIA_OVERRIDE_HTTP_PORT
#define KASIA_SERVER_HTTP_PORT KASIA_OVERRIDE_HTTP_PORT
#else
#define KASIA_SERVER_HTTP_PORT 80 // default http port
#endif

AsyncWebServer kasiaServer(KASIA_SERVER_HTTP_PORT);

#define SERVER_URI "https://dev-http-client.bromleysat.space"

void Kasia::startServer()
{
    kasiaServer.on("/", HTTP_GET2, [](AsyncWebServerRequest *request)
                   {
        HTTPClient http;

        http.begin(SERVER_URI);
        http.addHeader("v", KASIA_VERSION);
        http.addHeader("i", encode(_deviceId).c_str());

        if (!_dataConfig.empty())
        {
            http.addHeader("d", _dataConfig.c_str());
            Serial.println(_dataConfig.c_str());
        }

        //TODO full actions config
        http.addHeader("a", "UHVtcFdhdGVy");

        // TODO remove this and test failure scenarios
        http.addHeader("h", WiFi.localIP().toString());

        int httpResponseCode = http.GET();

        String payload;

        if (httpResponseCode == HTTP_CODE_OK)
        {
            payload = http.getString();
        }
        else
        {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
            payload = "Error: Please wait a bit and try to reload the page";
        }
        http.end();
    
        std::string t = SERVER_URI;
        t.append("/_next");
        payload.replace("/_next", t.c_str());
        t.erase();

        auto *response = request->beginResponse(200, "text/html", payload);
        //auto *response = request->beginResponseStream(200, "text/html");

        //TODO these cache headers do not work and can try meta tags
        //might need to return 304 when etag is the same
        //might need to use if-none-match header
        response->addHeader("cache-control", "public, s-maxage=36000, stale-while-revalidate=31536000");
        request->send(response); });


    kasiaServer.onNotFound([](AsyncWebServerRequest *request)
                           {
        Serial.println("404 for all");
        request->send(404); });

    kasiaServer.on("/d", HTTP_GET2, [](AsyncWebServerRequest *request)
                   {
        auto currentTimestamp = esp_timer_get_time();

        if(request->params() != (size_t)1){
            request->send(400, "text/plain", "Unexpected number of parameters"); 
            return;
        }
        auto isFirstElement = true;

        int64_t t = std::strtoull(request->getParam(0)->value().c_str(),NULL,0);
        
        //TODO test with reset/restart of ESP32 and how the data chart gets confused if at all
        //TODO same test for logs 
        std::string data = std::to_string(currentTimestamp);
        data.append("|");

        for (auto &&v : dataVectors)
        {
            if(!isFirstElement) data.append(",");
            else isFirstElement = false;  
            data.append(v());
        }

        auto filteredLogs = kasiaLog.FilteredLogs(t);

        if(filteredLogs.size()> 0)
        {
            std::string logsText = "|";
            auto isFirstLog = true;

            for (auto &&log : filteredLogs)
            {
                if(!isFirstLog) logsText.append(",");
                else isFirstLog = false; 

                logsText.append(std::to_string(currentTimestamp - log.Timestamp));
                logsText.append(encode(log.Text));                        
            }

            data.append(logsText);
        }
      
        request->send(200, "text/plain", String(data.c_str())); });

    int index = 0;
    std::string strIndex = "/";
    strIndex.append(std::to_string(index));

    kasiaServer.on(strIndex.c_str(), HTTP_POST, [](AsyncWebServerRequest *request)
                   {
        auto currentTimestamp = esp_timer_get_time();

        // auto action = actions.at("testAction");
        // action();

        request->send(204); });

    kasiaServer.begin();

    // TODO handle it cleaner with logger
    std::string str = _deviceId;
    str.append(" server started!");
    kasiaLog.Info(str);
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
        //  log_e("STA Started");
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

    // log_e("   ||");

    int temp = res;

    // BromleySatIoT::printLn(temp);
    //  std::string s = std::to_string(temp);
    //  log_e(s);

    if (res == ESP_OK)
    {
        // log_e("ESP_OK");
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

    // log_e("||   ");

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

    kasiaLog.SetConfig(baud, true);
    Kasia::startWiFi();
}

/**
 * print text line to serial TODO
 * @param text
 */
void Kasia::start(const char *deviceId, long baud)
{
    _deviceId = deviceId;
    kasiaLog.SetConfig(baud, true);
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
