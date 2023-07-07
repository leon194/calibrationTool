#pragma once

#include <string>

#include <b3ddepth/core/B3DDef.h>

struct B3DCalibrationData
{

    float leftCamIntrinsic[4];      // fx, fy, cx, cy
    float leftCamDistCoeff[5];      // k1, k2, p1, p2, k3
    float leftCamImageSize[2];      // width, height

    float rightCamIntrinsic[4];     // fx, fy, cx, cy
    float rightCamDistCoeff[5];     // k1, k2, p1, p2, k3
    float rightCamRotation[9];      // rotation matrix
    float rightCamTranlation[3];    // translation vector
    float rightCamImageSize[2];     // width, height

    float midCamIntrinsic[4];       // fx, fy, cx, cy
    float midCamDistCoeff[5];       // k1, k2, p1, p2, k3
    float midCamRotation[9];        // rotation matrix
    float midCamTranlation[3];      // translation vector
    float midCamImageSize[2];       // width, height

};

// read a binary file to B3DCalibrationData
DLLEXPORT bool readB3DCalibrationData(
    const std::string& inputDir, 
    const std::string& fileName,
    B3DCalibrationData& calibData
);

// write B3DCalibrationData to a binary file
DLLEXPORT bool writeB3DCalibrationData(
    const std::string& outputDir, 
    const std::string& fileName, 
    const B3DCalibrationData& calibData
);