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

#include <b3ddepth/core/B3DImage.h>

namespace b3dd {

    // Currently only saves/loads B3DIMAGE_GRAY to/from PNG image format
    bool DLLEXPORT loadB3DImage(const std::string& filepath, B3DImage& outImage, B3DImageType type = B3DIMAGE_MONO);

    bool DLLEXPORT saveB3DImage(const std::string& filepath, const B3DImage& image);


    // Debugging tools
    bool DLLEXPORT loadB3DImageBinary(const std::string& filepath, B3DImage& outImage);

    bool DLLEXPORT saveB3DImageBinary(const std::string& filepath, const B3DImage& image);

}  // End of namespace b3dd
