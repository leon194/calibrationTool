
#pragma once

#include <string>

#include "B3D4ExportDef.h"

/**
* @brief  An error struct for representing possible errors from DepthCamera
*
* Detailed descriptions:
*/

namespace b3d4 {

struct DLLEXPORT B3DNativeProcessError {
public:
    enum ErrorCode {
        B3D_NO_ERROR,                                           /**< Default is no error */

        /* File I/O Errors*/
        B3D_FILE_MISSING,                                       /**< file not found error */
        B3D_FILE_READ_FAIL,                                     /**< file read error */
        B3D_FILE_WRITE_FAIL,                                    /**< file write error */
        B3D_FILE_FORMAT_NOT_SUPPORTED,                          /**< file not support error */

        /* Depth Processing Errors */
        B3D_DEPTH_INPUT_IMAGE_INVALID,                          /**< Input depth Image invalid */
        B3D_DEPTH_INTPUT_CALIBATION_DATA_INVALID,               /**< Input calibration data invalid  */
        B3D_DEPTH_INPUT_ROI_INVALID,                            /**< Input ROI invalid  */
        B3D_DEPTH_INTPUT_RECT_MAP_INVALID,                      /**< Input rect map invalid */
        B3D_DEPTH_INPUT_CONFIG_INVALID,                         /**< Input config invalid  */
        /* Merge Processing Errors */
        B3D_SINGLE_VIEW_MERGE_SETTINGS_INVALID,                 /**< Input setting invalid  */
        B3D_SINGLE_VIEW_MERGE_INPUT_DEPTH_INVALID,              /**< Input depth invalid  */
        B3D_SINGLE_VIEW_MERGE_INPUT_CALIBRATION_DATA_INVALID,   /**< Input calibration data invalid  */
        B3D_SINGLE_VIEW_MERGE_FAILED,                           /**< merge failed  */
        /* Recalibration Processing Errors */
        B3D_RECALIBRATION_INPUT_IMAGE_INVALID,                  /**< Input depth Image invalid */
        B3D_RECALIBRATION_CALIB_SIZE_INVALID,                   /**< Input calib size  invalid */
        B3D_RECALIBRATION_CALIB_NO_CHANGE,                      /**< No need to change calib data */
        B3D_RECALIBRATION_CALIB_NO_ERROR,                      /**< Recalibration no error */
        /* Find LandMark Errors */
        B3D_FINDLANDMARK_INITTRACKER_INVALID,                   /**< Init tracker invalid */
        B3D_FINDLANDMARK_FACE_NOT_FOUND,                        /**< can't find face */

        
        B3D_OPENCV_ERROR,                                       /**< opencv error */
        B3D_OTHER_ERROR,                                         /**< other error */
    };

    // Default constructor
	B3DNativeProcessError(ErrorCode error = B3D_NO_ERROR, std::string message = "") :
            errorCode(error),
            debugMessage(message) {}

    ErrorCode errorCode;  /**< an enum that represents the error */

    std::string debugMessage;  /**< A string that describes the error debug message */

};
}