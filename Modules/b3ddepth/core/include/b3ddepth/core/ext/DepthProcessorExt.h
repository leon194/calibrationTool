/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 6/01/2018    jingliu created
//
//M*/
#pragma once

#include <b3ddepth/core/DepthProcessor.h>
#include <b3ddepth/core/ext/DepthConfigExt.h>

namespace b3dd {

    class DepthProcessorExt : public DepthProcessor {

    public:
        DLLEXPORT DepthProcessorExt();
        DLLEXPORT virtual ~DepthProcessorExt();

        DLLEXPORT virtual bool computeDepthExt(const B3DImage& imageStereo, B3DImage& outDepthMap);

        DLLEXPORT virtual bool computeRectsAndDisparityExt(
            const B3DImage& imageL, const B3DImage& imageR,
            B3DImage& rectL, B3DImage& rectR, B3DImage& outDisp
        );

        DLLEXPORT virtual DepthConfigExtPtr getProcessorSettingsExt() const;

        DLLEXPORT virtual void updateProcessorSettingsExt(DepthConfigExtPtr depthConfigPtr);

        // Debugging functions
        DLLEXPORT virtual void printTimingStatistics();

        DLLEXPORT void computeIRMask(const B3DImage& inImage, B3DImage& maskImage);
    };


}  // End of namespace b3dd
