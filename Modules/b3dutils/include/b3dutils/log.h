//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//

#pragma once

#include <string>
#include <iomanip>
#include <sstream>
#include <codecvt>

#include "SourceLocation.h"

namespace b3dutils {

    //!
    //! List of available log levels.
    //!
    enum LogLevel
    {
      LOG_DEBUG,
      LOG_INFO,
      LOG_WARNING,
      LOG_ERROR,
    };

    //!
    //! Sets log level.
    //!
    void DLLEXPORT SetLogLevel(LogLevel level);

    //!
    //! Returns current log level.
    //!
    LogLevel DLLEXPORT GetLogLevel();

    //!
    //! Returns true if a given log level is active.
    //!
    bool DLLEXPORT IsLogLevelActive(LogLevel level);

    //!
    //! Enables or disables B3D library logging.
    //!
    //void DLLEXPORT SetB3DLibraryLogEnabled(bool enabled);

    //!
    //! Returns whether B3D library log is enabled.
    //!
    bool DLLEXPORT IsB3DLibraryLogEnabled();

    //!
    //! Log message.
    //!
    void DLLEXPORT Log(LogLevel level, const SourceLocation& location, const std::string& message);

    //!
    //! Converts UNICODE string to UTF8 string.
    //!
    inline DLLEXPORT std::wstring StringToWideString(const std::string& string)
    {
      using codecvt_type = std::codecvt_utf8<wchar_t>;
      using convert_type = std::wstring_convert<codecvt_type, wchar_t>;

      convert_type converter;
      return converter.from_bytes(string);
    }

    //!
    //! Converts UTF8 string to UNICODE string.
    //!
    inline DLLEXPORT std::string WideStringToString(const std::wstring& string)
    {
      using codecvt_type = std::codecvt_utf8<wchar_t>;
      using convert_type = std::wstring_convert<codecvt_type, wchar_t>;

      convert_type converter;
      return converter.to_bytes(string);
    }

    //!
    //! Class allows usage of C++ output streams for writing log messages.
    //!
    template<LogLevel Level>
    class DLLEXPORT LogStream final
    {
    public:

      //!
      //! Default constructor.
      //!
      LogStream()
       : m_location{}
       , m_stream{}
      {}

      //!
      //! Constructs stream object and writes header information.
      //!
      LogStream(const SourceLocation& location)
        : m_location{ location }
        , m_stream{}
      {}

      //!
      //! Move constructor.
      //!
      LogStream(LogStream&&) = default;

      //!
      //! Destructor will write content of the stream buffer to log.
      //!
      ~LogStream() noexcept(false)
      {
        Log(Level, m_location, m_stream.str());
      }

      //!
      //! Write data to the stream buffer.
      //!
      template<typename ArgumentType>
      std::ostream& operator << (ArgumentType&& argument)
      {
        return m_stream << std::forward<ArgumentType>(argument);
      }

      //!
      //! Returns true is stream is active. 
      //!
      bool isActive() const noexcept
      {
        return IsLogLevelActive(Level);
      }

      //!
      //! Returns true is stream is active. 
      //!
      explicit operator bool() const noexcept
      {
        return IsLogLevelActive(Level);
      }

    private:
      SourceLocation m_location;
      std::ostringstream m_stream;
    };

    using LogStreamD = LogStream<LogLevel::LOG_DEBUG>;
    using LogStreamI = LogStream<LogLevel::LOG_INFO>;
    using LogStreamW = LogStream<LogLevel::LOG_WARNING>;
    using LogStreamE = LogStream<LogLevel::LOG_ERROR>;

} // namespace b3dutils

#define B3DLOGD if (auto stream = ::b3dutils::LogStreamD{ SOURCE_LOCATION }) stream
#define B3DLOGI if (auto stream = ::b3dutils::LogStreamI{ SOURCE_LOCATION }) stream
#define B3DLOGW if (auto stream = ::b3dutils::LogStreamW{ SOURCE_LOCATION }) stream
#define B3DLOGE if (auto stream = ::b3dutils::LogStreamE{ SOURCE_LOCATION }) stream
