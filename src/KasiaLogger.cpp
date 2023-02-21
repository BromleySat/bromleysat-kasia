#include "KasiaLogger.h"



KasiaLogger::KasiaLogger()
{
}

/**
 * Add log entry to internal in-memory collection
 * @param text
 * Text of the log entry
 */
void KasiaLogger::addLogEntry(std::string text)
{
  if (_printToSerial)
    Serial.println(text.c_str());

  if (!_printToWeb)
    return;

  if (logs.size() > KASIA_MAX_LOGS)
    logs.erase(logs.begin());
  logs.push_back(LogEntry(esp_timer_get_time(), text));
}

/**
 * print text line to serial
 * @param baud
 * Baud rate of the serial log.
 * If equal to 0 and greater then this enables logs to serial on the device
 * @param enableWebLogs
 * If set to true then enables logging to the web page or to the server
 */
void KasiaLogger::SetConfig(long baud, bool enableWebLogs)
{
  if (baud > -1)
  {
    _printToSerial = true;
    Serial.begin(baud);
  }

  _printToWeb = enableWebLogs;
}

bool KasiaLogger::IsPrintingToSerial()
{
  return _printToSerial;
}

/**
 * Add log entry of Information level
 * @param text
 * Text of the log entry
 */
void KasiaLogger::Info(const char *text)
{
  addLogEntry(text);
}

/**
 * Add log entry of Information level
 * @param text
 * Text of the log entry
 */
void KasiaLogger::Info(std::string text)
{
  addLogEntry(text);
}


template <typename... TArgs>
/**
 * Add log entry of Information level
 * @param args
 * Multiple elements that are concatenated as a single string
 */
void KasiaLogger::Info(TArgs... args)
{
  std::stringstream s;
  const int dummy[] = {0, (s << std::forward<TArgs>(args), 0)...};
  static_cast<void>(dummy); // avoid warning for unused variable
 
  addLogEntry(s.str());
}

/**
 * Returns filtered list of log entries
 * @param timestamp
 * Timestamp of the point in time from when to take logs
 */
std::vector<LogEntry> KasiaLogger::FilteredLogs(int64_t timestamp)
{
  std::vector<LogEntry> filteredLogs;
  std::copy_if(logs.begin(), logs.end(), std::back_inserter(filteredLogs), [&](LogEntry &x)
               { return x.Timestamp > timestamp; });
  return filteredLogs;
}

KasiaLogger kasiaLog;
