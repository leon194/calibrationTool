//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//

#pragma once

#include <ostream>

#include "B3DUtilsExportDef.h"

#include "macros.h"

namespace b3dutils {

    //!
    //! The SourceLocation class represents certain information about 
    //! the source code, such as file names, line numbers, and function names.
    //!
    class DLLEXPORT SourceLocation final
    {
    public:
      using LineType = unsigned;
      using FileType = const char*;
      using FunctionType = const char*;

      //!
      //! Creates empty SourceLocation object.
      //!
      constexpr SourceLocation() noexcept
        : m_function{ nullptr }
        , m_file{ nullptr }
        , m_line{ 0 }
      {}

      //!
      //! Creates SourceLocation object from given funtion name, file name and line number.
      //! SOURCE_LOCATION macro can be used for easier construction of this object. 
      //!
      constexpr SourceLocation(FunctionType function, FileType file, LineType line) noexcept
        : m_function{ function }
        , m_file{ file }
        , m_line{ line }
      {}

      //!
      //! Returns function name.
      //!
      constexpr FunctionType getFunction() const noexcept
      {
        return m_function ? m_function : "";
      }

      //!
      //! Returns file name.
      //!
      constexpr FileType getFile() const noexcept
      {
        return m_file ? m_file : "";
      }

      //!
      //! Returns line number.
      //!
      constexpr LineType getLine() const noexcept
      {
        return m_line;
      }

      //!
      //! Returns true if function name is empty.
      //!
      constexpr bool isFunctionEmpty() const noexcept
      {
        return nullptr == m_function;
      }

      //!
      //! Returns true if file name is empty.
      //!
      constexpr bool isFileEmpty() const noexcept
      {
        return nullptr == m_file;
      }

      //!
      //! Returns true if line number is empty.
      //!
      constexpr bool isLineEmpty() const noexcept
      {
        return 0 == m_line;
      }

      //!
      //! Returns true if SourceLocation object is empty.
      //!
      constexpr bool isEmpty() const noexcept
      {
        return nullptr == m_function
          && nullptr == m_file
          && 0 == m_line;
      }

      //!
      //! Returns true if SourceLocation object is not empty. 
      //!
      constexpr explicit operator bool() const noexcept
      {
        return nullptr != m_function
          || nullptr != m_file
          || 0 != m_line;
      }

    private:
      FunctionType m_function;
      FileType m_file;
      LineType m_line;
    };

    //!
    //! Write SourceLocation object to the output stream.
    //!
    std::ostream& operator << (std::ostream& stream, const SourceLocation& location);

} // namespace b3dutils

#define SOURCE_LOCATION ::b3dutils::SourceLocation{  SOURCE_FUNCTION,  SOURCE_FILE,  SOURCE_LINE }
