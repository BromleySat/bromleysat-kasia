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
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_http_server.h>
#include "esp_task_wdt.h"
} // extern "C"

std::vector<Kasia::TFuncVoidString> dataVectors;
std::string Kasia::_dataConfig = "";
const char *Kasia::_deviceId = "Kasia Device";
KasiaCachedClient *Kasia::_client = nullptr;
TaskHandle_t Kasia::_kasiaOverwatchTaskHandle = NULL;

AsyncWebServer kasiaServer(KASIA_SERVER_HTTP_PORT);

#define SERVER_URI "https://kasia-http-demo.bromleysat.space"

#define MAX_WIFI_ATTEMPTS 4

Kasia::TActionBoolCharPtr Kasia::onGotIP = [](bool isNew, const char *ip)
{
    if (!isNew)
    {
        logInfo("WiFi Reconnected");
        return;
    }

    if (!kasiaLog.IsPrintingToSerial())
        return;
    Serial.print("Connected got new IP: ");
    Serial.println(ip);
};

Kasia::Kasia()
{
}

/**
 * Binds data so that it can be accessed by reference later on
 * This changes the config that is sent by the device
 * @param label
 * Data name or label
 * @param ptr
 * Pointer to the data element
 */
void Kasia::bindData(const char *label, float *ptr)
{
    bindData(label, KASIA_TYPE_FLOAT, [=]()
             { return std::to_string(*ptr); });
}

/**
 * Binds data so that it can be accessed by reference later on
 * This changes the config that is sent by the device
 * @param label
 * Data name or label
 * @param ptr
 * Pointer to the data element
 */
void Kasia::bindData(const char *label, bool *ptr)
{
    bindData(label, KASIA_TYPE_BOOL, [=]()
             { return std::to_string((int16_t)*ptr); });
}

/**
 * Binds data so that it can be accessed by reference later on
 * This changes the config that is sent by the device
 * @param label
 * Data name or label
 * @param ptr
 * Pointer to the data element
 */
void Kasia::bindData(const char *label, int *ptr)
{
    bindData(label, KASIA_TYPE_INT32, [=]()
             { return std::to_string(*ptr); });
}

/**
 * Reusable protected method to bind data
 * @param label
 * Data name or label
 * @param type
 * Data type as in text format to avoid the need for conversion later on
 * @param valueLambda
 * Lambda function that will convert current value of the data to string
 */
void Kasia::bindData(const char *label, const char *type, Kasia::TFuncVoidString valueLambda)
{
    if (!_isFirstElement)
        _dataConfig.append("|");
    else
        _isFirstElement = false;

    _dataConfig.append(type);
    _dataConfig.append(encode(label));
    dataVectors.push_back(valueLambda);
}

/**
 * Check if the device is currently connected to WiFi
 * @return
 * Returns true if connected, otherwise false
 */
bool Kasia::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

/**
 * Blocking call to wait until the device is connected to WiFi. Returns to true if successful
 * @param timeout
 * Timeout in milliseconds to wait for connection. If takes longer than this, will stop trying and will return false
 */
bool Kasia::waitUntilConnected(ulong timeout)
{
    int8_t notConnectedAttempts = 0;
    auto prevStatus = WL_IDLE_STATUS;
    auto startTime = millis();

    while (timeout == ULONG_MAX || (millis() - startTime) < timeout)
    {
        if (Kasia::isConnected())
            return true;

        if (WiFi.status() == WL_NO_SSID_AVAIL)
        {
            if (prevStatus != WL_NO_SSID_AVAIL)
            {
                logInfo("WiFi connection error: WiFi network with the specified SSID is not available");
                notConnectedAttempts = 0;
                prevStatus = WL_NO_SSID_AVAIL;
            }
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
        else if (WiFi.status() == WL_DISCONNECTED)
        {
            if (notConnectedAttempts > MAX_WIFI_ATTEMPTS)
            {
                if (prevStatus != WL_DISCONNECTED)
                {
                    logInfo("WiFi connection error: The WiFi network is there but the password may well be incorrect");
                    prevStatus = WL_DISCONNECTED;
                }
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
            else
                notConnectedAttempts++;
        }
        else if (WiFi.status() == WL_CONNECT_FAILED)
        {
            if (notConnectedAttempts > MAX_WIFI_ATTEMPTS)
            {
                if (prevStatus != WL_CONNECT_FAILED)
                {
                    logInfo("WiFi connection error: Connection failed and will try to reset connection");
                    prevStatus = WL_CONNECT_FAILED;
                    notConnectedAttempts = 0;
                }
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
            else
                notConnectedAttempts++;
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    return false;
}

/**
 * Blocking call to wait until the device is connected to WiFi
 */
void Kasia::waitUntilConnected()
{
    waitUntilConnected(ULONG_MAX);
}

/**
 * Start Async Web Server
 */
void Kasia::startServer()
{
    kasiaServer.on("/", HTTP_GET2, [](AsyncWebServerRequest *request)
                   {

        String payload = _client->get().c_str();
    
        std::string t = SERVER_URI;
        t.append("/_next");
        payload.replace("/_next", t.c_str());
        t.erase();

        auto *response = request->beginResponse(200, "text/html", payload);
        response->addHeader("cache-control", "public, s-maxage=36000, stale-while-revalidate=31536000");
        request->send(response); });

    kasiaServer.on("/d", HTTP_GET2, [](AsyncWebServerRequest *request)
                   {
        auto currentTimestamp = esp_timer_get_time();

        if(request->params() != (size_t)1){
            request->send(400, "text/plain", "Unexpected number of parameters"); 
            return;
        }
        auto isFirstElement = true;

        int64_t t = std::strtoull(request->getParam(0)->value().c_str(),NULL,0);
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

    kasiaServer.begin();
    logInfo(_deviceId, " server started!");
}

bool connected = false;

static std::string getReason(wifi_err_reason_t reason)
{
    switch (reason)
    {
    case WIFI_REASON_UNSPECIFIED:
        return "of ¯\\_(ツ)_/¯. Most likely the WiFi point is down or unreachable";
    case WIFI_REASON_AUTH_EXPIRE:
        return "authentication has expired";
    case WIFI_REASON_AUTH_LEAVE:
        return "authentication had to 'leave' apparently ¯\\_(ツ)_/¯";
    case WIFI_REASON_ASSOC_EXPIRE:
        return "association has expires";
    case WIFI_REASON_ASSOC_TOOMANY:
        return "there are too many associations";
    case WIFI_REASON_NOT_AUTHED:
        return "the connection was not authenticated";
    case WIFI_REASON_NOT_ASSOCED:
        return "the connection was not associated";
    case WIFI_REASON_ASSOC_LEAVE:
        return "association had to 'leave' apparently ¯\\_(ツ)_/¯";
    case WIFI_REASON_ASSOC_NOT_AUTHED:
        return "the association was not authenticated";
    case WIFI_REASON_DISASSOC_PWRCAP_BAD:
        return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
    case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
        return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
    case WIFI_REASON_BSS_TRANSITION_DISASSOC:
        return "of BSS transition disassociation";
    case WIFI_REASON_IE_INVALID:
        return "WIFI_REASON_IE_INVALID";
    case WIFI_REASON_MIC_FAILURE:
        return "of MIC failure";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        return "four-way handshake has timed out";
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
        return "group key update has timed out";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS:
        return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
    case WIFI_REASON_GROUP_CIPHER_INVALID:
        return "the group cipher was not valid";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
        return "the pairwise cipher was not valid";
    case WIFI_REASON_AKMP_INVALID:
        return "AKMP was not valid";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
        return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
    case WIFI_REASON_INVALID_RSN_IE_CAP:
        return "WIFI_REASON_INVALID_RSN_IE_CAP";
    case WIFI_REASON_802_1X_AUTH_FAILED:
        return "802 1X Authentication has failed";
    case WIFI_REASON_CIPHER_SUITE_REJECTED:
        return "cipher suite was rejected";
    case WIFI_REASON_INVALID_PMKID:
        return "PMKID was not valid";
    case WIFI_REASON_BEACON_TIMEOUT:
        return "the beacon has timed out";
    case WIFI_REASON_NO_AP_FOUND:
        return "no AP was found";
    case WIFI_REASON_AUTH_FAIL:
        return "authentication has failed";
    case WIFI_REASON_ASSOC_FAIL:
        return "association has failed";
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
        return "the handshake has timed out";
    case WIFI_REASON_CONNECTION_FAIL:
        return "connection has failed";
    case WIFI_REASON_AP_TSF_RESET:
        return "of AP transfer reset";
    case WIFI_REASON_ROAMING:
        return "'roaming' ¯\\_(ツ)_/¯";
    default:
        return "Unknown";
    }
}

void Kasia::_arduino_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    arduino_event_t arduino_event;
    arduino_event.event_id = ARDUINO_EVENT_MAX;

    /*
     * STA
     * */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        log_v("STA Started");
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
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE;
        memcpy(&arduino_event.event_info.wifi_sta_authmode_change, event_data, sizeof(wifi_event_sta_authmode_change_t));
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_CONNECTED;
        memcpy(&arduino_event.event_info.wifi_sta_connected, event_data, sizeof(wifi_event_sta_connected_t));
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_DISCONNECTED;
        memcpy(&arduino_event.event_info.wifi_sta_disconnected, event_data, sizeof(wifi_event_sta_disconnected_t));

        if (connected)
        {
            logInfo("WiFi disconnected because ",
                    getReason((wifi_err_reason_t)arduino_event.event_info.wifi_sta_disconnected.reason),
                    ". Attempting to reconnect");
            connected = false;
        }

        kasia.reconnect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        if (event->ip_changed)
        {
            _client->setHeader("h", IPAddress(IP2STR(&event->ip_info.ip)).toString().c_str());
        }
        Kasia::onGotIP(event->ip_changed, IPAddress(IP2STR(&event->ip_info.ip)).toString().c_str());
        connected = true;
        kasia.stopOverwatch();
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_GOT_IP;
        memcpy(&arduino_event.event_info.got_ip, event_data, sizeof(ip_event_got_ip_t));
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        logInfo("WiFi IP Lost");
        arduino_event.event_id = ARDUINO_EVENT_WIFI_STA_LOST_IP;
        kasia.reconnect();
    }
    else
    {
        if (event_id != 21)
            logInfo("Unknown event. EventBase:", event_base, " EventId:", event_id);
    }
}

void Kasia::overwatchTask(void *param)
{
    while (true)
    {
        kasia.waitUntilConnected();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void Kasia::startOverwatch()
{
    if (!_kasiaOverwatchTaskHandle)
    {
        xTaskCreatePinnedToCore(overwatchTask, "KasiaWiFiOverwatch", 4096, NULL, 1, &Kasia::_kasiaOverwatchTaskHandle, ARDUINO_EVENT_RUNNING_CORE);
        if (!_kasiaOverwatchTaskHandle)
        {
            logInfo("Kasia WiFi Overwatch Task Start Failed!");
        }
    }
}

void Kasia::stopOverwatch()
{
    if (_kasiaOverwatchTaskHandle)
    {
        vTaskDelete(_kasiaOverwatchTaskHandle);
        _kasiaOverwatchTaskHandle = NULL;
    }
}

/**
 * Start WiFi connection using credentials that were set previously
 * Non blocking call
 */
void Kasia::startWiFi()
{
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        log_e("esp_event_loop_create_default failed!");
        return; // err;
    }

    auto res = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Kasia::_arduino_event_cb, NULL, NULL);

    if (esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &_arduino_event_cb, NULL, NULL))
    {
        log_e("event_handler_instance_register for IP_EVENT Failed!");
    }

    //  start connection. This returns the result but we do not wait for it
    if (_ssid == NULL || _pwd == NULL)
    {
        // this will use WiFi credentials that were saved at previous conenction attempt
        WiFi.begin();
    }
    else
    {
        WiFi.begin(_ssid, _pwd);
    }

    startOverwatch();

    _client = new KasiaCachedClient(SERVER_URI);
    _client->setHeader("v", KASIA_VERSION);
    _client->setHeader("i", encode(_deviceId).c_str());

    if (!_dataConfig.empty())
    {
        _client->setHeader("d", _dataConfig.c_str());
    }

    startServer();
}

void Kasia::reconnect()
{
    WiFi.reconnect();
    //Restarting overwatch creates WiFi connection issues
    //Can either keep it running permanently or a single use only
    //startOverwatch();
}

/**
 * Start Kasia framework while specifying deviceId, baud rate and WiFi credentials
 * @param deviceId
 * Unique Id or name of the device
 * @param baud
 * Serial baud rate. eg 9600
 * @param ssid
 * WiFi network name
 * @param pwd
 * WiFi password
 */
void Kasia::start(const char *deviceId, long baud, const char *ssid, const char *pwd)
{
    if (_isStartCalled)
        return; // defensive programming
    _isStartCalled = true;

    _deviceId = deviceId;
    _ssid = ssid;
    _pwd = pwd;

    kasiaLog.SetConfig(baud, true);
    Kasia::startWiFi();
}

/**
 * Start Kasia framework while specifying deviceId and baud rate
 * This can be used if the device was previously started successfully with valid WiFi credentials
 * @param deviceId
 * Unique Id or name of the device
 * @param baud
 * Serial baud rate. eg 9600
 */
void Kasia::start(const char *deviceId, long baud)
{
    if (_isStartCalled)
        return; // defensive programming
    _isStartCalled = true;

    _deviceId = deviceId;
    kasiaLog.SetConfig(baud, true);
    Kasia::startWiFi();
}

/**
 * Start Kasia framework while specifying deviceId
 * This can be used if the device was previously started successfully with valid WiFi credentials
 * @param deviceId
 * Unique Id or name of the device
 */
void Kasia::start(const char *deviceId)
{
    if (_isStartCalled)
        return; // defensive programming
    _isStartCalled = true;

    _deviceId = deviceId;
    Kasia::startWiFi();
}

/**
 * Start Kasia framework without specifying any configuration
 * This can be used if the device was previously started successfully with valid WiFi credentials
 */
void Kasia::start()
{
    if (_isStartCalled)
        return; // defensive programming
    _isStartCalled = true;

    Kasia::startWiFi();
}

Kasia kasia;
