#include "KasiaLogger.h"

KasiaLogger::KasiaLogger()
{
}

/**
 * print text line to serial
 * @param text
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
 * @param text
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
 * print text line to serial
 * @param text
 */
void KasiaLogger::Info(const char *text)
{
  addLogEntry(text);
}

/**
 * print text line to serial
 * @param text
 */
void KasiaLogger::Info(std::string text)
{
  addLogEntry(text);
}

/**
 * print text line to serial
 * @param text
 */
template <class... TArgs>
void KasiaLogger::Info(TArgs &&...args)
{
  const char *texts[] = {args...};
  std::string text = "";
  for (auto &&t : texts)
  {
    text.append(t);
    // text.append(std::to_string(t));
  }

  addLogEntry(text);
}

/**
 * print text line to serial
 * @param text
 */
std::vector<LogEntry> KasiaLogger::FilteredLogs(int64_t timestamp)
{
  std::vector<LogEntry> filteredLogs;
  std::copy_if(logs.begin(), logs.end(), std::back_inserter(filteredLogs), [&](LogEntry &x)
               { return x.Timestamp > timestamp; });
  return filteredLogs;
}

KasiaLogger kasiaLog;
