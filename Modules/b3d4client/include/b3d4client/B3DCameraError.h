/*M///////////////////////////////////////////////////////////////////////////////////////
//
// DepthCameraError represents possible error code from DepthCamera class
//
// Copyright (c) 2019 Bellus3D, Inc. All rights reserved.
//
// 9/6/2017	jingl created
//M*/

#pragma once

#include <string>

#include "B3D4ExportDef.h"

/**
* @brief  An error struct for representing possible errors from DepthCamera
*
* Detailed descriptions:
*/
namespace b3d4 {

    struct DLLEXPORT B3DCameraError {

        enum ErrorCode {
            B3D_NO_ERROR,            /**< Default is no error */
            FILE_IO_ERROR,           /**< Fails to read, write or delete a file or folder */
            FILE_MISSING,            /**< Cannot read a specified file */
            INVALID_STATE_ERROR,     /**< Methods called at invalid state: Call appropriate method first */
            CONNECTION_LOST,         /**< Camera disconnected: Plug in camera */
            NOT_CLOSED_PROPERLY,     /**< APP exits before closing camera properly */
            INTERNAL_LIBRARY_ERROR,  /**< Error occurred inside B3D internal library */
            SERVICE_NOT_RUNNING      /**< InuService not running: Plug in camera to start service */
        };

        ErrorCode errorCode;  /**< an enum that represents the error */

        std::string debugMessage;  /**< A string that describes the error debug message */

        // Default constructor
        B3DCameraError(ErrorCode error = B3D_NO_ERROR, std::string message = "") :
            errorCode(error),
            debugMessage(message) {}

    };

}  // End of namespace b3d