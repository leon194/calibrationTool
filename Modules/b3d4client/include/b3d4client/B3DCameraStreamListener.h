#pragma once

#include <memory>

#include <opencv2/core.hpp>
//#include "B3DImage.h"

#include <b3d4client/B3DNativeProcessError.h>
#include <b3di/FaceTracker.h>

#include "B3D4ExportDef.h"


namespace b3d4 {

// Used with frame time stamp in millisecond
typedef unsigned long FrameTime;
typedef unsigned long FrameID;

typedef cv::Mat FrameImage;

/** Information and non-image data about a frame */
class DLLEXPORT FrameInfo {
public:

	enum FrameType {
		PREVIEW = 0,  // for preview
		CAPTURE = 1   // for capture
	};

    FrameInfo(FrameID id = 0, FrameTime t = 0) {
        frameTime = t; frameId = id;
    }

    /** Unique frame id **/
    FrameID frameId;

    /** Frame time offset from the start of a session in millisecond **/
    FrameTime frameTime;

	/** Frame type, preview or capture **/
	FrameType frameType;
    /**
    * Check if the frame is empty
    * @return True if the frame is empty
    */
    bool empty() const { return frameId == 0; }

};

class DLLEXPORT B3DCameraFrame {
public:

	// Each device may have up to 3 camera
	enum FrameType {
		L_FRAME = 0,  // IR left  camera (from camera view facing the world)
		R_FRAME = 1,  // IR right camera
		M_FRAME = 2,  // color    camera
        D_FRAME = 3,  // Depth Image Frame
        STEREO_FRAME = 4,  // LR interleaved frame
	};

    B3DCameraFrame(FrameType type) {
        frameType = type;
        isValid   = false;
    }

    // TODO: do we need default values for these fields?
    FrameType  frameType;  // indicates which camera
    FrameInfo  frameInfo;
    FrameImage frameImage;

    bool isValid;  // do we still need this flag?
};
using B3DCameraFramePtr = std::shared_ptr<B3DCameraFrame>;

class DLLEXPORT B3DCameraStreamListener {
public:
    virtual ~B3DCameraStreamListener() {}

    virtual void onFrame(B3DCameraFramePtr framePtr) = 0;
};
using B3DCameraStreamListenerPtr = std::shared_ptr<B3DCameraStreamListener>;

/* a listener notify process status */
class DLLEXPORT B3DCameraProcessListener {
public:
    virtual ~B3DCameraProcessListener() {}

    virtual void onError(B3DNativeProcessError error) {};

    virtual void onFrameEnough(std::vector<cv::Mat> &colorFrame,int keyFrameNum, int keyFrameOffset) {};

    virtual void onProcessDone(std::vector<cv::Mat> &depthFrame, cv::Rect &faceRect) {};

    virtual void onStitcherDone(
              cv::Mat &depthImage,
              cv::Mat &confidenceMaps,
              cv::Mat &rmseMaps,
              cv::Mat &irMask) {};

    virtual void onFaceDetectDone(std::vector<float> &headPoseInfo) {};

	virtual void onGenFaceLandMarkDone(cv::Ptr<b3di::FaceTracker>& ft) {};

	virtual void onRecalibrationDone(B3DNativeProcessError error, float calibDispErr) {};

	virtual void onStreamDepthDone(cv::Mat &depthFrame, long timeStampL) {};

    virtual void onProcessFinished(std::string who) {};
};

using B3DCameraProcessListenerPtr = std::shared_ptr<B3DCameraProcessListener>;



} // namespace b3d
