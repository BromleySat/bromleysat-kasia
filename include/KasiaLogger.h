#ifndef KASIA_LOGGER_H
#define KASIA_LOGGER_H

#include <Arduino.h>
#include <string>
#include <vector>

#ifdef KASIA_OVERRIDE_LOG_RETENTION
#define KASIA_MAX_LOGS KASIA_OVERRIDE_LOG_RETENTION - 1
#else
#define KASIA_MAX_LOGS 9 // 10 logs max
#endif

struct LogEntry
{
  int64_t Timestamp;
  std::string Text;
  LogEntry(int64_t timestamp, std::string text) : Timestamp(timestamp), Text(text) {}
};

class KasiaLogger
{

public:
  KasiaLogger();
  void SetConfig(long baud, bool enableWebLogs);
  bool IsPrintingToSerial();
  void Info(const char *text);
  void Info(std::string text);
  template <class... TArgs>
  void Info(TArgs &&...args);
  std::vector<LogEntry> FilteredLogs(int64_t timestamps);

protected:
  bool _printToSerial = false;
  bool _printToWeb = true;
  std::vector<LogEntry> logs;
  void addLogEntry(std::string text);
};

extern KasiaLogger kasiaLog;

#endif /* KASIA_LOGGER_H */
