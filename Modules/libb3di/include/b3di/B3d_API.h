//
// Copyright (c) 2019 Bellus3D, Inc. All rights reserved.
//

#pragma once

#include <string>
#include <iostream>
#include <sstream>

#include "opencv2/core.hpp"

namespace b3di {
  //!
  //! Major version number.
  //!
  extern const int VersionMajor;

  //!
  //! Minor version number.
  //!
  extern const int VersionMinor;

  //!
  //! Patch version number.
  //!
  extern const int VersionPatch;

  //!
  //! Hash of the git commit (40 characters).
  //!
  extern const char* const VersionCommit;

  //!
  //! Date and time of the git commit.
  //!
  extern const char* const VersionDate;

  //!
  //! Short version string.
  //!
  extern const char* const VersionShort;

  //!
  //! Long version string.
  //!
  extern const char* const VersionLong;

  //!
  //! Name of the build pipeline.
  //!
  extern const char* const BuildPipeline;

  //!
  //! Date and time of the build.
  //!
  extern const char* const BuildDate;

  //!
  //! Name of the build.
  //!
  extern const char* const BuildName;

  //!
  //! Unique identifier of the build.
  //!
  extern const int BuildUUID;

  class B3D_API {

  public:

      static cv::String getVersionString() {
          std::ostringstream os;
          os << VersionMajor << "." << VersionMinor << "." << VersionPatch;
          return os.str();
      }

  };

} // namespace b3di
