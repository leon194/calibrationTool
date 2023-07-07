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

namespace b3dd
{
    // For saving 8bit gray scale depth image
    void DLLEXPORT convert16To8(const B3DImage& inputMap, B3DImage& outMap);

    // For displaying IR image on Android
    // converts a single channel gray scale to 4 channel RGBA
    void DLLEXPORT convert8UTo8UC4(const B3DImage& inImage, B3DImage& outImage);

    // For displaying gray scale Depth Image
    void DLLEXPORT convert16UTo8UC4(const B3DImage& inImage, B3DImage& outImage);

    // For displaying Rainbow visualization Depth Image
    void DLLEXPORT convert16UTo8UC4RainbowColor(const B3DImage& inImage, B3DImage& outImage);

    // For saving Rainbow visualization to file
    void DLLEXPORT convert16UTo8URainbowColor(const B3DImage& inImage, B3DImage& outImage);

    // For saving Heatmap visualization to file
    void DLLEXPORT convert16UToHeatmap(const B3DImage& inImage, B3DImage& outImage, bool isDynamic = false);

    // For saving Heatmap visualization to file
    void DLLEXPORT convert16UToC4Heatmap(const B3DImage& inImage, B3DImage& outImage, bool isDynamic = false);

    // Visualize depth map as normal maps
    void DLLEXPORT convert16UToNormalmap(const B3DImage& inImage, B3DImage& outImage, bool isC4 = false);
}  // End of namespace b3dd
