#pragma once

namespace b3di {

class TLog
{
public:
	typedef enum
	{
		LOG_ALL=0,
		LOG_VERBOSE=1,
		LOG_INFO=2,
		LOG_WARNING=3,
		LOG_ERROR=4,
		LOG_ALWAYS=5,			// Force to always print unless logLevel is LOG_NONE
		LOG_NONE=10,
	} LogType;
	static void setLogLevel(LogType type);
	static LogType getLogLevel();
	static void setLogFile(const char* fileName, bool deleteOld=true);
	static void log(LogType type, const char* strFormat, ...);

protected:
	static LogType s_logType;
	static char s_logFile[256];

};

} // namespace b3di