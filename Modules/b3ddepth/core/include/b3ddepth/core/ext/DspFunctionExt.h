/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 6/01/2018    jingliu created
//
//M*/
#pragma once

#include <b3ddepth/core/B3DImage.h>
#include <b3ddepth/core/CameraCalib.h>
#include <b3ddepth/core/B3DDef.h>

//#define RUN_QVGA

namespace b3dd {

    DLLEXPORT void stereobm_DSP(const B3DImage& imageL, const B3DImage& imageR, B3DImage& dispImage);

    DLLEXPORT void stereobm_DSP_TwoDispMaps(const B3DImage& imageL, const B3DImage& imageR, B3DImage& dispImage1,B3DImage& dispImage2);

    void stereobm_DSP_without_pad(const B3DImage& imageLImg,
                                  const B3DImage& imageRImg,
                                  B3DImage& dispImg);

    DLLEXPORT void rectification_DSP(const B3DImage& imageL, const B3DImage& imageR, CameraCalibPtr cameraCalibPtr,
        B3DImage& rectImgL, B3DImage& rectImgR);

    DLLEXPORT void disparityToDepth_DSP(const B3DImage& dispImage, CameraCalibPtr cameraCalibPtr,
        B3DImage& outDepthImage);

    DLLEXPORT void test_functions();


    DLLEXPORT void tmp_resize(const B3DImage& inImage, B3DImage& outImage);

    typedef std::pair<int, int> mypair;

}  // End of namespace b3dd
