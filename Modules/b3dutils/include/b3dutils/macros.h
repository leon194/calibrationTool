//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//

#pragma once

#include <memory>

///////////////////////////////////////////////////////////////////////////////

#define STRINGIFY_IMPL(TOKEN) #TOKEN
#define STRINGIFY(TOKEN) STRINGIFY_IMPL(TOKEN)

///////////////////////////////////////////////////////////////////////////////

#define CONCATENATE_IMPL(FIRST, SECOND) FIRST##SECOND
#define CONCATENATE(FIRST, SECOND) CONCATENATE_IMPL(FIRST, SECOND)

///////////////////////////////////////////////////////////////////////////////

#if defined(__COUNTER__)
#  define ANONYMOUS_VARIABLE(NAME) CONCATENATE(NAME, __COUNTER__)
#else
#  define ANONYMOUS_VARIABLE(NAME) CONCATENATE(NAME, __LINE__)
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#  define SOURCE_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define SOURCE_FUNCTION CONCATENATE(__FUNCTION__, "()")
#else
#  define SOURCE_FUNCTION CONCATENATE(__func__, "()")
#endif

#if defined(SOURCE_LOCATION_LENGTH)
# define SOURCE_FILE ((sizeof(__FILE__)/sizeof(char) > SOURCE_LOCATION_LENGTH) ? (__FILE__ + SOURCE_LOCATION_LENGTH) : __FILE__)
#else
# define SOURCE_FILE __FILE__
#endif

#define SOURCE_LINE __LINE__

///////////////////////////////////////////////////////////////////////////////

#define DECLARE_SMART_POINTERS(TYPE)                          \
  class TYPE;                                                 \
  using CONCATENATE(TYPE, Ptr) = std::shared_ptr<TYPE>;       \
  using CONCATENATE(TYPE, WeakPtr) = std::weak_ptr<TYPE>;     \
  using CONCATENATE(TYPE, UniquePtr) = std::unique_ptr<TYPE>