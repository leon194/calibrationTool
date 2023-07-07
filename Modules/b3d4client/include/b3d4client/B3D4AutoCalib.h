/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2017 Bellus3D, Inc. All rights reserved.
//
// 8/15/2017	sec	created
//
//M*/
// 
#pragma once

#include "b3di/CameraParams.h"

#include "b3ddepth/core/ext/CameraCalibExt.h"
#include "b3ddepth/core/ext/DepthProcessorExt.h"

namespace b3d4 {


/** Class for automatic re-calibration */
class DLLEXPORT B3D4AutoCalib {
public:

    B3D4AutoCalib();
    virtual ~B3D4AutoCalib();

    /**
    * Compute disp error from input IR images
    *
    * @param [in] calibDataFilePath Path to calib.data file
    * @param [in] imageL
    * @param [in] imageR
    * @param [in] camL
    * @param [in] camR
    * @param [in] depthCamType The depth camera type. Use the stereoBaseline for baseline checking.
    * @param [in] roiRect The rectangular image region to optimize the calibration for. If not defined, optimize for the entire image
    * @param [out] dispError Disparity error
    * @param [out] number of valid feature points
    * @return true if compute disp error is successful
    *
    */
    virtual bool computeDispError(const cv::Mat& imageL, const cv::Mat& imageR,
        const b3di::CameraParams& camL, const b3di::CameraParams& camR,
        b3dd::DepthCameraType depthCamType, cv::Rect2f roiRect, float& dispError, 
        int& validFeaturesCount);


    /**
    * Recalibarte an existing calibration from input IR images
    * Only works if the calibration has changed slightly in intrinisc cy params
    * Assume all other calibration parameters remain mostly the same.
    * Only re-calibrate L/R cameras, not M camera.
    * camL and camR should contain existing camera params as input.
    * On return, they contain the recalibrated camera params.
    * This funciton uses stereo blocking matching
    * and blockSize, minDisparity, and maxDisparity are params for StereoBM
    *
    * @param [in] imageL
    * @param [in] imageR
    * @param [in/out] camL
    * @param [in/out] camR
    * @param [in] blockSize block size for stereo block matching
    * @param [in] minDisparity minimum disparity value for stereo block matching
    * @param [in] maxDisparity maximum disparity value for stereo block matching
    * @param [in] maxCyOffset max cy search range in pixels. The returned cy will be no more than maxCyOffset from the original value.
    * @param [in] roiRect The rectangular image region to optimize the calibration for. If not defined, optimize for the entire image.
    * @param [in] depthCamType The depth camera type. Use the stereoBaseline for baseline checking.
    * @return true if recalibration is successful
    *
    */
    virtual bool recalibrate(const cv::Mat& imageL, const cv::Mat& imageR,
        b3di::CameraParams& camL, b3di::CameraParams& camR, bool& calibChanged,
        b3dd::DepthCameraType depthSDKCamType = b3dd::DepthCameraType::DEPTHCAM_B3D4,
        int maxCyOffset = 3, cv::Rect2f roiRect = cv::Rect2f(),
        int depthCamType = -1, float errorThr = 0.0f);
};


} // namespace b3d4