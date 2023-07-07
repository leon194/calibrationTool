/*M///////////////////////////////////////////////////////////////////////////////////////
//
// FrameInfo defines meta data type for a captured frame image
//
// Copyright (c) 2017 Bellus3D, Inc. All rights reserved.
//
// 1/31/2017	sec	created
//
//M*/
// 
#pragma once

#include <opencv2/core.hpp>

namespace b3di {

// Used with frame time stamp in millisecond
typedef unsigned long FrameTime;
typedef unsigned long FrameID;

/** Information and non-image data about a frame */
class FrameInfo {
public:
	FrameInfo(FrameID id = 0, FrameTime t = 0)
	{
		frameTime = t; frameId = id;
	}

	FrameInfo(const cv::String& fileName)
	{
		getFrameInfoFromFileName(fileName);
	}

	/** Unique frame id **/
	FrameID frameId;

	/** Frame time offset from the start of a session in millisecond **/
	FrameTime frameTime;

	/** Parse a file name to get frame data **/
	void getFrameInfoFromFileName(const cv::String& fileName);

	/**
	* Check if the frame is empty 
	* @return True if the frame is empty
	*/
	bool empty() const { return frameId == 0; }

};


} // namespace b3di