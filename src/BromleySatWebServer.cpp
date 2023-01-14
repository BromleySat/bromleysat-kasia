
#include <BromleySatWebServer.h>
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

bool BromleySatWebServer::_printToSerial = false;

BromleySatWebServer::THandlerFunction BromleySatWebServer::test = []()
{
    Serial.println("static test");
};

BromleySatWebServer::TActionBoolCharPtr BromleySatWebServer::onGotIP = [](bool isNew, const char *ip)
{
    BromleySatWebServer::println("Got ", isNew ? "new" : "same", " IP: ", ip);
};

BromleySatWebServer::BromleySatWebServer()
{
}

/**
 * print text line to serial
 * @param text
 */
void BromleySatWebServer::print(const char *text)
{
    if (_printToSerial)
        Serial.print(text);
}

/**
 * print text line to serial
 * @param text
 */
void BromleySatWebServer::println(const char *text)
{
    if (_printToSerial)
        Serial.println(text);
}

/**
 * print text line to serial
 * @param text
 */
template <class... TArgs>
void BromleySatWebServer::println(TArgs &&...args)
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
void BromleySatWebServer::temp(const char *label, float *ptr)
{
    String temp = label;
    temp += " : ";
    temp += String(*ptr);
    Serial.println(temp);
}

/**
 * print text line to serial
 * @param text
 */
bool BromleySatWebServer::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

/**
 * print text line to serial
 * @param text
 */
void BromleySatWebServer::waitUntilConnected()
{
    while (true)
    {
        if (BromleySatWebServer::isConnected())
            break;
        // TODO can't use delay
        delay(1);
    }
}

/**
 * print text line to serial
 * @param text
 */
bool BromleySatWebServer::waitUntilConnected(unsigned int timeout)
{
    // get start time

    // get cuurrent time

    while (true)
    {
        if (BromleySatWebServer::isConnected())
            return true;
        // TODO can't use delay
        delay(1);
    }

    return false;
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

        BromleySatWebServer::onGotIP(event->ip_changed, IPAddress(IP2STR(&event->ip_info.ip)).toString().c_str());

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
void BromleySatWebServer::setBaud(long baud)
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
// void BromleySatWebServer::onGotIP(void onMsg (bool isNew, const char *ip))
// {
//     //(&onMsg->)(true,"test");
// }
/**
 * print text line to serial TODO
 * @param text
 */
void BromleySatWebServer::startWiFi()
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
}

/**
 * print text line to serial TODO
 * @param text
 */
void BromleySatWebServer::start(long baud, const char *ssid, const char *pwd)
{
    _ssid = ssid;
    _pwd = pwd;

    BromleySatWebServer::setBaud(baud);
    BromleySatWebServer::startWiFi();
}

/**
 * print text line to serial TODO
 * @param text
 */
void BromleySatWebServer::start(long baud)
{
    BromleySatWebServer::setBaud(baud);
    BromleySatWebServer::startWiFi();
}

/**
 * print text line to serial TODO
 * @param text
 */
void BromleySatWebServer::start()
{
    BromleySatWebServer::startWiFi();
}

BromleySatWebServer web;
