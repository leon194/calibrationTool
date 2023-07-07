/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2017 Bellus3D, Inc. All rights reserved.
//
// 9/7/2017	sec	created
//
//M*/
#pragma once

#include <opencv2/core.hpp>
#include "TLog.h"

namespace b3di {

// This enum maps to DepthCameraSettings::DepthCameraType
enum DepthCamType {
	DEPTHCAM_B3D2,
	DEPTHCAM_B3D3,
	DEPTHCAM_SPRD,
	DEPTHCAM_IPHX,
	DEPTHCAM_ORB50,
	DEPTHCAM_INU25,
	DEPTHCAM_VIVOT,
	DEPTHCAM_B3D4,
	NUM_DEPTHCAMS
};

// Should match DepthCamType above
static const cv::String DepthCamTypeName[NUM_DEPTHCAMS] = {
	"B3D2",
	"B3D3",
	"SPRD",
	"IPHX",
	"ORB50",
	"INU25",
	"VIVOT",
	"B3D4"
};

inline cv::String getDepthCamTypeString(DepthCamType depthCamType) {

	int index = (int)depthCamType;
	if (index >= 0 && index < NUM_DEPTHCAMS) {
		return DepthCamTypeName[index];
	}
	TLog::log(TLog::LOG_WARNING, "getDepthCamTypeString() - unknown depth cam type %d", depthCamType);
	return "";

}

inline DepthCamType getDepthCamType(const cv::String& typeString) {

	for (int i = 0; i < NUM_DEPTHCAMS; i++) {
		if (DepthCamTypeName[i] == typeString) {
			return (DepthCamType)i;
		}
	}

	TLog::log(TLog::LOG_WARNING, "getDepthCamType - unknown depth cam type name %s", typeString.c_str());
	return b3di::NUM_DEPTHCAMS;
}


struct DepthCamProps {
	double minScanDistance;		// in mm
	double maxScanDistance;		// in mm
	double faceDetectionWindowWidthRatio;		// Ratio (0-1) to image width for the center face detection window
	double faceDetectionWindowHeightRatio;		// Ratio (0-1) to image height for the center face detection window
	double stereoBaseline;		// in mm (set to 0 for non-stereo cameras)
	int stereoBlockSize;			// in number of pixels of the stereo block width/height (set to 0 for non-stereo cameras)

	int trackingImageHeight;		// Target IR image height for real time face pose tracking; recale the input IR images if necessary. If the height is the same as the output depth map size, the post processing doesn't need to recompute the depth and is more efficient
//	int mcTimeOffset;			// timestamp offset between M and C cam frames (in ms)

	int depthSmoothingMin;		// control minimum amount of smoothing. Typical value is 4. Should be between 2-6.
	int depthSmoothingFactor;		// control amount of smoothing. Typically value is 30. Larger value is more smoothing

	int depthBorderMagin;		// Depth border pixels tend to be inaccurate so this controls the amount of image border to remove. Typical value is 8-16. N/A if depth images are provided

	double depthUnitScale;			// for converting depth values (16-bit uint) to z values (mm)
	double depthUnitOffset;			// z = depthUnitScale* d + depthUnitOffset

	double depthMergeThreshold;		// in mm. depth threshold for merging multiple depth images. The threshold should be related to the depth noise (larger value for noisy depth)
	double depthSmoothThreshold;		// NOT USED. in mm. depth threshold for smoothing merged depth


	cv::Vec3d imageScales;		// IR image scale ratios for LD, MD and HD.

	cv::Size cylMapSize_ld;
	cv::Size textureMapSize_ld;

	cv::Size cylMapSize_sd;
	cv::Size textureMapSize_sd;
	
	cv::Size cylMapSize_hd;
	cv::Size textureMapSize_hd;

};

static const DepthCamProps DEPTHCAM_PROPS[NUM_DEPTHCAMS] = {
#ifdef _SQUARE_MAP
	{ 250, 450, 0.8, 0.8, 50.0, 19, 480, 1, 25, 10, 0.02, 0, 8.0, 8.0, cv::Vec3d(0.75, 0.75, 0.75), cv::Size(1280, 1280), cv::Size(1024, 1024), cv::Size(1280, 1280), cv::Size(4096, 4096), cv::Size(1280, 1280), cv::Size(4096, 4096) },
	{ 250, 450, 0.9, 0.9, 50.0, 19, 480, 1, 25, 10, 0.02, 0, 8.0, 8.0, cv::Vec3d(0.75, 0.75, 0.75), cv::Size(1280, 1280), cv::Size(1024, 1024), cv::Size(1280, 1280), cv::Size(4096, 4096), cv::Size(1280, 1280), cv::Size(4096, 4096) },
	{ 250, 400, 0.85, 0.85, 30.0, 11, 320, 1, 25, 10, 0.02, 0, 5.0, 8.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 960), cv::Size(1024, 1024), cv::Size(960, 960), cv::Size(4096, 4096), cv::Size(960, 960), cv::Size(4096, 4096) },
	{ 250, 400, 0.85, 0.85, 0.0, 0, 320, 1, 25, 6, 0.018522, -406.642, 5.0, 8.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 960), cv::Size(1024, 1024), cv::Size(960, 960), cv::Size(4096, 4096), cv::Size(960, 960), cv::Size(4096, 4096) },
	{ 250, 400, 0.85, 0.85, 0.0, 0, 480, 1, 25, 6, 0.01, 0, 5.0, 8.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 960), cv::Size(1024, 1024), cv::Size(960, 960), cv::Size(4096, 4096), cv::Size(960, 960), cv::Size(4096, 4096) },
	{ 250, 450, 0.85, 0.85, 25.0, 11, 480, 1, 25, 6, 0.02, 0, 5.0, 8.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 960), cv::Size(1024, 1024), cv::Size(960, 960), cv::Size(4096, 4096), cv::Size(960, 960), cv::Size(4096, 4096) },
	{ 250, 450, 0.85, 0.85, 0.0, 0, 320, 1, 25, 6, 0.25, 0, 5.0, 8.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 960), cv::Size(1024, 1024), cv::Size(960, 960), cv::Size(4096, 4096), cv::Size(960, 960), cv::Size(4096, 4096) },
	{ 200, 450, 0.85, 0.85, 40.0, 23, 480, 1, 25, 15, 0.02, 0, 8.0, 8.0, cv::Vec3d(0.75, 0.75, 0.75), cv::Size(1280, 1280), cv::Size(1024, 1024), cv::Size(1280, 1280), cv::Size(4096, 4096), cv::Size(1280, 1280), cv::Size(4096, 4096) },

#else
	{ 250, 450, 0.8, 0.8, 50.0, 19, 480, 2, 30, 8, 0.02, 0, 3.0, 7.0, cv::Vec3d(0.75, 0.75, 0.75), cv::Size(1440, 720), cv::Size(2048, 1024), cv::Size(1440, 720), cv::Size(4096, 2048), cv::Size(1440, 720), cv::Size(4096, 2048) },
	{ 250, 450, 0.9, 0.9, 50.0, 19, 480, 2, 30, 8, 0.02, 0, 3.0, 7.0, cv::Vec3d(0.75, 0.75, 0.75), cv::Size(1440, 720), cv::Size(2048, 1024), cv::Size(1440, 720), cv::Size(4096, 2048), cv::Size(1440, 720), cv::Size(4096, 2048) },
	{ 250, 450, 0.85, 0.85, 30.0, 11, 320, 2, 30, 4, 0.02, 0, 3.0, 7.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 480), cv::Size(1024, 512), cv::Size(960, 480), cv::Size(4096, 2048), cv::Size(960, 480), cv::Size(4096, 2048) },
	{ 250, 430, 0.85, 0.85, 0.0, 0, 320, 4, 30, 0, 0.018522, -406.642, 5.0, 7.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 480), cv::Size(1024, 512), cv::Size(960, 480), cv::Size(4096, 2048), cv::Size(960, 480), cv::Size(4096, 2048) },
	{ 250, 400, 0.85, 0.85, 0.0, 0, 480, 2, 20, 1, 0.01, 0, 5.0, 7.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 480), cv::Size(1024, 512), cv::Size(960, 480), cv::Size(4096, 2048), cv::Size(960, 480), cv::Size(4096, 2048) },
	{ 250, 450, 0.85, 0.85, 25.0, 11, 480, 2, 20, 3, 0.02, 0, 3.0, 7.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 480), cv::Size(1024, 512), cv::Size(960, 480), cv::Size(4096, 2048), cv::Size(960, 480), cv::Size(4096, 2048) },
	{ 250, 450, 0.85, 0.85, 0.0, 0, 480, 4, 35, 1, 0.25, 0, 10.0, 7.0, cv::Vec3d(1.0, 1.0, 1.0), cv::Size(960, 480), cv::Size(1024, 512), cv::Size(960, 480), cv::Size(4096, 2048), cv::Size(960, 480), cv::Size(4096, 2048) },
	{ 200, 450, 0.85, 0.85, 40.0, 23, 480, 2, 30, 8, 0.02, 0, 3.0, 7.0, cv::Vec3d(0.75, 0.75, 0.75), cv::Size(1440, 720), cv::Size(2048, 1024), cv::Size(1440, 720), cv::Size(4096, 2048), cv::Size(1440, 720), cv::Size(4096, 2048) },
#endif
};

}