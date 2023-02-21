#ifndef KASIA_LOGGER_H
#define KASIA_LOGGER_H

#include <Arduino.h>
#include <string>
#include <vector>
#include <sstream>

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
  template <typename... TArgs>
  void Info(TArgs... args);
  std::vector<LogEntry> FilteredLogs(int64_t timestamps);

protected:
  bool _printToSerial = false;
  bool _printToWeb = true;
  std::vector<LogEntry> logs;
  void addLogEntry(std::string text);
};

extern KasiaLogger kasiaLog;

template <typename... TArgs>
void kasiaLogInfo(TArgs... args)
{
  std::stringstream s;
  const int dummy[] = {0, (s << std::forward<TArgs>(args), 0)...};
  static_cast<void>(dummy); // avoid warning for unused variable
  
  // Need to dynamically enable for C++17
  //  std::stringstream s;
  //  (s << ... << std::forward<TArgs>(args));
  kasiaLog.Info(s.str());
}

template <typename... TArgs>
/**
 * Add log entry of Information level
 * @param args
 * Multiple elements that are concatenated as a single string
 */
void logInfo(TArgs... args)
{
  kasiaLogInfo(args...);
}

#endif /* KASIA_LOGGER_H */
