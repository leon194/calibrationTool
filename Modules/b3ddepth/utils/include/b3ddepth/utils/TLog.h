/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 6/01/2018    jingliu created
//
//M*/
#pragma once

#include <string>
#include <string.h>  // do we need this one?

#include <b3ddepth/core/B3DDef.h>

namespace b3dd {

class DLLEXPORT TLog
{
public:
    typedef enum
    {
        LOG_ALL     =  0,
        LOG_VERBOSE =  1,
        LOG_INFO    =  2,
        LOG_WARNING =  3,
        LOG_ERROR   =  4,
        LOG_ALWAYS  =  5,			// Force to always print unless logLevel is LOG_NONE
#ifdef ANDROID
        LOG_STACKTRACE = 6,
#endif
        LOG_NONE    = 10,
    } LogType;

	static void setLogLevel(LogType type);
	static LogType getLogLevel();
	static void setLogFile(const char* fileName, bool deleteOld=true);
	static void log(LogType type, const char* strFormat, ...);
	static int getProcessId();
	static int getThreadId();

protected:
	static LogType s_logType;
	static char s_logFile[256];
};

} // namespace b3dd

#if defined(WIN32) || defined(TARGET_OS_IPHONE)

#define logAll(format, ...)                    \
        b3dd::TLog::log(b3dd::TLog::LOG_ALL, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logVerbose(format, ...)                \
        b3dd::TLog::log(b3dd::TLog::LOG_VERBOSE, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logInfo(format, ...)                   \
        b3dd::TLog::log(b3dd::TLog::LOG_INFO, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logWarning(format, ...)                \
        b3dd::TLog::log(b3dd::TLog::LOG_WARNING, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logError(format, ...)                  \
        b3dd::TLog::log(b3dd::TLog::LOG_ERROR, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logAlways(format, ...)                 \
        b3dd::TLog::log(b3dd::TLog::LOG_ALWAYS, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#elif defined(ANDROID)

#define logAll(fmt, args...)                    \
        b3dd::TLog::log(b3dd::TLog::LOG_ALL, "[%d-%d] [%s] [%d] " \
        fmt, b3dd::TLog::getProcessId(), b3dd::TLog::getThreadId(), __func__, __LINE__,##args)

#define logVerbose(fmt, args...)                \
        b3dd::TLog::log(b3dd::TLog::LOG_VERBOSE, "[%d-%d] [%s] [%d] " \
        fmt, b3dd::TLog::getProcessId(), b3dd::TLog::getThreadId(), __func__, __LINE__,##args)

#define logInfo(fmt, args...)                   \
        b3dd::TLog::log(b3dd::TLog::LOG_INFO, "[%d-%d] [%s] [%d] " \
        fmt, b3dd::TLog::getProcessId(), b3dd::TLog::getThreadId(), __func__, __LINE__,##args)

#define logWarning(fmt, args...)                \
        b3dd::TLog::log(b3dd::TLog::LOG_WARNING, "[%d-%d] [%s] [%d] " \
        fmt, b3dd::TLog::getProcessId(), b3dd::TLog::getThreadId(), __func__, __LINE__,##args)

#define logError(fmt, args...)                  \
        b3dd::TLog::log(b3dd::TLog::LOG_ERROR, "[%d-%d] [%s] [%d] " \
        fmt, b3dd::TLog::getProcessId(), b3dd::TLog::getThreadId(), __func__, __LINE__,##args)

#define logAlways(fmt, args...)                 \
        b3dd::TLog::log(b3dd::TLog::LOG_ALWAYS, "[%d-%d] [%s] [%d] " \
        fmt, b3dd::TLog::getProcessId(), b3dd::TLog::getThreadId(), __func__, __LINE__,##args)

#define logStackTrace(fmt, args...)              \
        b3dd::TLog::log(b3dd::TLog::LOG_STACKTRACE, "[%d-%d] [%s] [%d] " \
        fmt, b3dd::TLog::getProcessId(), b3dd::TLog::getThreadId(), __func__, __LINE__,##args)

#else
#endif

void logTiming(std::string operation, double time_ms);
