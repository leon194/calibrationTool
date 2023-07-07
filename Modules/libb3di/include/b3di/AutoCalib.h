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

#include "CameraParams.h"
#include "FaceTracker.h"

namespace b3di {


/** Class for automatic re-calibration */
class AutoCalib {
public:

	AutoCalib();
	virtual ~AutoCalib();

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
	* @param [in] blockSize stereo matching block size
	* @param [in] maxCyOffset max cy search range in pixels. The returned cy will be no more than maxCyOffset from the original value.
	* @param [in] stereoBaseline stereo matching baseline distance. If not set, compuete it from camL and camR distance
	* @param [in] faceLandmarks Used to determine the face region for recalibration. Otherwise, use the entire image.
	* @return true if recalibration is successful
	*
	*/
	virtual bool recalibrate(const cv::Mat& imageL, const cv::Mat& imageR, 
		b3di::CameraParams& camL, b3di::CameraParams& camR, bool& calibChanged,
		int blockSize, int maxCyOffset=10, double stereoBaseline = 0,
		const FaceTracker::FaceLandmarks& faceLandmarks= FaceTracker::FaceLandmarks(),
		const b3di::CameraParams& inputCamM= b3di::CameraParams());

};


} // namespace b3di