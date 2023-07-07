/*M///////////////////////////////////////////////////////////////////////////////////////
//
// DepthCameraSettings.
//
// Copyright (c) 2017 Bellus3D, Inc. All rights reserved.
//
// 3/12/2017	sec	created
// This class manages the paths that will be used by B3D API
// Also the settings that will be loaded and used by B3D API
// Maybe even version numbers?
//M*/

#pragma once

#include <string>
#include <vector>
#include <memory>

//#include <opencv2/core.hpp>
//
//#ifdef TARGET_OS_IPHONE
//#include <b3d/B3DSettingsError.h>
//#else
//#include "B3DSettingsError.h"
//#endif

/** Main namespace for all B3D classes */
namespace b3d4 {

class DepthCameraSettings {

public:

    /**
    * An enum mapped to DepthCamType in "DepthCamProps.h"
    */
    enum class DepthCameraType {
        B3D2,  /**< enum value B3D2 add-on */
        B3D3,  /**< enum value B3D3 add-on */
        SPRD,  /**< enum value SPRD Phone */
        IPHX   /**< enum value iPhoneX */
    };

    /** Default constructor. */
    static std::shared_ptr<DepthCameraSettings> newDepthCameraSettings();

    /** Destructor. */
    virtual ~DepthCameraSettings() = 0;


    /**
    * @brief  Load settings from a configuration file from disk
    *
    * Where this file could be found is based on setPath?
    *
    * The Settings class itself should know what the filename is
    * Later on we might support loading non-default settings file
    *
    * @return bool
    */
    //virtual B3DSettingsError loadSettingsFromFile(const cv::String& filename) = 0;


    //virtual void setDepthCameraType(DepthCameraType depthCameraType) = 0;


    // virtual double getStereoBaseline() const = 0;


public:

    /**
    * DepthCameraSettings constructor, will load default values
    *
    */
    DepthCameraSettings() {}


};

using DepthCameraSettingsPtr = std::shared_ptr<DepthCameraSettings>;

}  // End of namespace b3d
