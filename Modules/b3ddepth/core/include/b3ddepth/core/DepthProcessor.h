/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 6/01/2018    jingliu created
//
//M*/
#pragma once

// B3D Depth Core
#include <b3ddepth/core/B3DImage.h>
#include <b3ddepth/core/CameraCalib.h>
#include <b3ddepth/core/DepthConfig.h>

namespace b3dd {

    class DepthProcessorImpl;
    using DepthProcessorImplPtr = std::shared_ptr<DepthProcessorImpl>;

    class DepthProcessor {
    public:
        DLLEXPORT DepthProcessor();
        DLLEXPORT virtual ~DepthProcessor();


        /**
        * @brief  Load calibration data for Depth Processor (should be done only once per instance)
        *
        * @param[in] CameraCalibPtr cameraCalib, camera calibration data
        * @return void
        *
        * @see CameraCalib::loadFromFile
        */
        DLLEXPORT virtual void loadCalibrationData(CameraCalibPtr calibDataPtr);


        /**
        * @brief  Update depth processing parameters.
        *
        * @param[in] DepthConfig is a class include
        *            OPTIMIZE_MODE optimizeMode, depth map optimized for speed or quality
        *            DEPTH_REGISTRATION depthRegistration, output depth map camera space
        *            float depthScale, output scale of depth map
        *            float depthUnit, output depth map units (default = 0.02, covers up to 3 meters)
        *            ROI ROI_L region of interest of L image
        * @return void
        *
        * @see CameraCalib::loadFromFile
        */
        DLLEXPORT virtual void updateProcessorSettings(DepthConfigPtr configPtr);


        /**
        * @brief  Compute depth from a pair of stereo IR images with active patterns
        *
        * "outDepthMap" is 16bit, in order to convert to millimeter, multiply a scale of 0.02
        *
        * @param[in] B3DImage imageL, the left camera image
        * @param[in] B3DImage imageR, the right camera image
        * @param[out] B3DImage outDepthMap, computed depth map
        * @return void
        */
        DLLEXPORT virtual bool computeDepth(const B3DImage& imageL, const B3DImage& imageR,
            B3DImage& outDepthMap);


        /**
        * @brief  Compute disparity from a pair of stereo IR images with active patterns
        *
        * "outDisp" is 16bit, in order to convert to millimeter, multiply a scale of 0.02
        *
        * @param[in] B3DImage imageL, the left camera image
        * @param[in] B3DImage imageR, the right camera image
        * @param[out] B3DImage outDisp, computed disparity
        * @return void
        */
        DLLEXPORT virtual bool computeDisparity(const B3DImage& imageL, const B3DImage& imageR,
            B3DImage& outDisp);


        /**
        * Obsolete, try to use "loadCalibrationData" & "updateProcessorSettings"
        *
        * @brief  Set depth processing parameters.
        *
        * @param[in] DepthProcessorParam is a class include
        * @param[in] CameraCalibPtr cameraCalib, camera calibration data
        * @param[in] OPTIMIZE_MODE optimizeMode, depth map optimized for speed or quality
        * @param[in] DEPTH_REGISTRATION depthRegistration, output depth map camera space
        * @param[in] float depthScale, output scale of depth map
        * @param[in] float depthUnit, output depth map units (default = 0.02, covers up to 3 meters)
        * @return void
        *
        * @see CameraCalib::loadFromFile
        */
        DLLEXPORT virtual void setProcessingParams(
            CameraCalibPtr cameraCalib,
            OPTIMIZE_MODE optimizeMode = OPTIMIZE_MODE::OPTIMIZE_QUARTER_RES,
            DEPTH_REGISTRATION depthRegistration = DEPTH_REGISTRATION::REGISTER_L,
            float depthScale = 1.0f, float depthUnit = 0.02f
        );


    protected:
        DepthProcessorImplPtr _impl;
    };


}  // End of namespace b3dd
