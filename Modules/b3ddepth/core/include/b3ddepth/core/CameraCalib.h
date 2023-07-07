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
#include <memory>

#include <b3ddepth/core/B3DImage.h>

#define CALIB_DATA_VERSION 0.2

namespace b3dd
{
    class CameraCalibImpl;
    using CameraCalibImplPtr = std::shared_ptr<CameraCalibImpl>;

    class CameraCalib {

    public:
        DLLEXPORT CameraCalib();
        DLLEXPORT virtual ~CameraCalib();

        /**
        * @brief  Load calibration data from a file.
        *
        * @param[in] std::string filepath, the path to calibration data file
        * @return false on error
        */
        DLLEXPORT virtual bool loadFromFile(std::string filepath);

        /**
        * @brief  Read byte stream stored on sensor OTP, and cache calibration data to disk
        *
        * @param[in] unsigned char*, byte stream that contains calibration parameters
        * @param[in] std::string filepath, the path to calibration data file
        * @return false on error
        */

        DLLEXPORT virtual bool cacheCalibrationData(unsigned char* data, std::string outputFilepath);

        /**
        * @brief  Recalibrate stereo IR cameras to account for slight hardware changes
        *
        * @param[in] B3DImage imageL the left camera image
        * @param[in] B3DImage imageR the right camera image
        * @return false on error
        */
        DLLEXPORT virtual bool recalibrate(const B3DImage& imageL, const B3DImage& imageR);

        DLLEXPORT virtual bool isEmpty();

        // Return the implementation of CameraCalib. For internal use only.
        CameraCalibImplPtr getImpl();

    protected:
        CameraCalibImplPtr _impl;

    };
    using CameraCalibPtr = std::shared_ptr<CameraCalib>;

}  // End of namespace b3dd
