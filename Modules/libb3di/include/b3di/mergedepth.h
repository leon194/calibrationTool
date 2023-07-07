#pragma once

#include <vector>

// include opencv
#include <opencv2/core.hpp>

// include b3di
#include "CameraParams.h"
#include "DepthCamProps.h"

namespace b3di
{

struct MergeDepth_Input
{

    std::vector<cv::Mat> inputDepthImages;      // input depth images
    b3di::CameraParams depthCam;                // input depth cam params
    b3di::DepthCamType depthCamType;            // depth cam type

    int keyFrameIdx;                            // depth data will be align to key depth image
    int minSampleNum;                           // min sample num for filtering

    cv::Mat headMask;                           // head mask for icp alignment
};

struct MergeDepth_Output
{
    cv::Mat depthImage;   // stiched depth output
    //cv::Mat keyColorImage;                      // the color image corresponding to the key depth image
    cv::Mat confidenceMap;
    cv::Mat rmseMap;
    cv::Mat irMask;
};

bool mergeDepth(const MergeDepth_Input& input, MergeDepth_Output& output);

}
