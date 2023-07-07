//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//

#include "SourceLocation.h"

///////////////////////////////////////////////////////////////////////////////

std::ostream& b3dutils::operator << (std::ostream& stream, const SourceLocation& location)
{
  if (location.isEmpty()) {
    return stream << "{}";
  }

  return stream << "{ " 
    << location.getFunction() << " : " 
    << location.getFile() << " : " 
    << location.getLine() 
    << " }";
}
