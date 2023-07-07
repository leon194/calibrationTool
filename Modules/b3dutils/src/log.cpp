//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//

#include <log.h>
//#include <Constants.hpp>
//#include <B3DLog.h>
#include <iostream>
#include <thread>
#include <mutex>

///////////////////////////////////////////////////////////////////////////////

namespace {
  std::mutex g_mutex;
  bool g_B3DLogEnabled;
  b3dutils::LogLevel g_level;
} // namespace

///////////////////////////////////////////////////////////////////////////////

namespace {
  const struct LogInitializer {
    LogInitializer() {
      b3dutils::SetLogLevel(b3dutils::LogLevel::LOG_INFO);
    }
  } g_initializer;
} // namespace

///////////////////////////////////////////////////////////////////////////////

void b3dutils::SetLogLevel(LogLevel level) {
//std::lock_guard<std::mutex> lock{ g_mutex };
//g_level = level;


    //if (!g_B3DLogEnabled) {
    //    b3d::SetLogLevel(b3d::LogLevel::LOG_NONE);
    //}
    //else {
    //    switch (g_level) {
    //    case LogLevel::LOG_INFO: b3d::SetLogLevel(b3d::LogLevel::LOG_INFO);    break;
    //    case LogLevel::LOG_WARNING: b3d::SetLogLevel(b3d::LogLevel::LOG_WARNING); break;
    //    case LogLevel::LOG_ERROR: b3d::SetLogLevel(b3d::LogLevel::LOG_ERROR);   break;
    //    }
    //}
}

///////////////////////////////////////////////////////////////////////////////

b3dutils::LogLevel b3dutils::GetLogLevel()
{
  std::lock_guard<std::mutex> lock{ g_mutex };
  return g_level;
}

///////////////////////////////////////////////////////////////////////////////

bool b3dutils::IsLogLevelActive(LogLevel level)
{
  std::lock_guard<std::mutex> lock{ g_mutex };
  return level >= g_level;
}

///////////////////////////////////////////////////////////////////////////////

//void b3dutils::SetB3DLibraryLogEnabled(bool enabled)
//{
//  std::lock_guard<std::mutex> lock{ g_mutex };
//  g_B3DLogEnabled = enabled;
//
//   if (!g_B3DLogEnabled) {
//     b3d::SetLogLevel(b3d::LogLevel::LOG_NONE);
//   } else {
//     switch (g_level) {
//       case LogLevel::LOG_INFO     : b3d::SetLogLevel(b3d::LogLevel::LOG_INFO);    break;
//       case LogLevel::LOG_WARNING  : b3d::SetLogLevel(b3d::LogLevel::LOG_WARNING); break;
//       case LogLevel::LOG_ERROR    : b3d::SetLogLevel(b3d::LogLevel::LOG_ERROR);   break;
//     }
//   }
//}

///////////////////////////////////////////////////////////////////////////////

bool b3dutils::IsB3DLibraryLogEnabled()
{
  std::lock_guard<std::mutex> lock{ g_mutex };
  return g_B3DLogEnabled;
}

///////////////////////////////////////////////////////////////////////////////

void b3dutils::Log(LogLevel level, const SourceLocation& location, const std::string& message)
{
  std::lock_guard<std::mutex> lock{ g_mutex };
  if (level >= g_level) {
    auto& stream = [&]() -> std::ostream& {
        switch (level) {
        case LogLevel::LOG_DEBUG    : return std::cout << "B3DLOG DEBUG";
        case LogLevel::LOG_INFO     : return std::cout << "B3DLOG INFO";
        case LogLevel::LOG_WARNING  : return std::cout << "B3DLOG WARNING";
        case LogLevel::LOG_ERROR    : return std::cerr << "B3DLOG ERROR";
      }
      return std::cout;
    } ();

//if (Constants::LogTimestamp) {
//    const auto timestamp = std::chrono::system_clock::now().time_since_epoch();
//    stream << " { Timestamp: " << std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count() << " } :";
//}
//
//if (Constants::LogThreadID) {
//    stream << " { Thread: " << std::this_thread::get_id() << " } :";
//}
//
//if (!location.isEmpty() && Constants::LogSourceLocation) {
//    stream << " at: " << location << " :";
//}

//if (!location.isEmpty()) {
//    stream << " at: " << location << " :";
//}

    stream << " " << message << "\n";
  }
}
