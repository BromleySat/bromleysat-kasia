/*
Licence and stuff

*/

#ifndef BROMLEYSAT_WIFI_H
#define BROMLEYSAT_WIFI_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

// template <typename ...T>
//     std::enable_if_t<(std::is_same_v<T, char>&&...)>

//template <class... TArgs>
 // std::enable_if_t<(std::is_same_v<T, char>&&...)>




class BromleySatWebServer
{
public:
  BromleySatWebServer();
  void temp(const char * label, float * ptr);
  void start(long baud, const char *ssid, const char *pwd);
  void start(long baud);
  void start();
  //TOOD either all prints are static or none are
    //might use a separate logger class global instance
  void print(const char *text);
  void println(const char *text);
   template <class... TArgs> static void println(TArgs &&...args);
  bool isConnected();
  void waitUntilConnected();
  bool waitUntilConnected(unsigned int timeout);

  typedef std::function<void(void)> THandlerFunction;
  typedef std::function<void(bool, const char *)> TActionBoolCharPtr;

//TODO encapsulation
  static THandlerFunction test;
  static TActionBoolCharPtr onGotIP;

protected:
  const char *_ssid;
  const char *_pwd;
  // const unsigned long _baud;
  static bool _printToSerial;
  void setBaud(long baud);
  void startWiFi();
  // void onGotIP(void onMsg (bool isNew, const char *ip));
};

extern BromleySatWebServer web;

#endif /* BROMLEYSAT_WIFI_H */
