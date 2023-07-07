/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 12/04/2018    John Zhong created
//
//M*/
#pragma once

#include <b3ddepth/core/CameraCalib.h>


namespace b3dd
{
    enum CAM_REGISTRATION {
        CAM_L,
        CAM_R,
        CAM_RGB
    };

    class CameraCalibImpl;
    using CameraCalibImplPtr = std::shared_ptr<CameraCalibImpl>;

    class CameraCalibExt : public CameraCalib {
    public:
        DLLEXPORT CameraCalibExt();
        DLLEXPORT virtual ~CameraCalibExt();

        DLLEXPORT virtual double getFx(const int cam = CAM_L) const;

        DLLEXPORT virtual double getFy(const int cam = CAM_L) const;

        DLLEXPORT virtual double getCx(const int cam = CAM_L) const;

        DLLEXPORT virtual double getCy(const int cam = CAM_L) const;

        DLLEXPORT virtual double getCalibDataVersion() const;

        DLLEXPORT virtual int getCameraType() const;

        /**
        * @brief  Read byte stream stored on sensor OTP, and cache calibration data to disk
        *
        * @param[in] unsigned char*, byte stream that contains calibration parameters
        * @param[in] std::string filepath, the path to calibration data file
        * @return false on error
        */

        DLLEXPORT virtual bool cacheCalibrationData(unsigned char* data, std::string outputFilepath, int camType);

        /**
        * @brief  Read byte stream stored on sensor OTP, and cache calibration data to disk
        *
        * @param[in] unsigned char*, byte stream that contains calibration parameters
        * @return false on error
        */

        DLLEXPORT virtual bool createCalibrationData(unsigned char* data, int camType);
    };

    using CameraCalibExtPtr = std::shared_ptr<CameraCalibExt>;

}  // End of namespace b3dd
