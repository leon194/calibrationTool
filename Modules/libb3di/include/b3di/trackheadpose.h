
#pragma once

#include <vector>

#include <opencv2/core.hpp>

#include "B3d_defs.h"
#include "CameraContainer.h"
#include "FrameStore.h"
#include "FaceTracker.h"
#include "DepthProcessor.h"
#include "DepthCamProps.h"

class HeadPoseIntData;

struct TrackHeadPose_Input {

	TrackHeadPose_Input() {

        wait = 0;
        show = false;
		verbose = false;
        computeHeadDepth = false;

		numThreads = 1;

		imageScale = 0.25f;
		//faceSearchRegionRatio = (float)b3di::PORTRAIT_CAM_FACE_REGION;
		//faceSearchRegionRatio = (float)b3di::DEPTHCAM_PROPS[b3di::DEPTHCAM_B3D2].faceDetectionWindowWidthRatio;
		maxICPError = (float)b3di::ICP_MAX_ERROR;

		depthCameraType = b3di::DEPTHCAM_B3D2;

		//inputCams = NULL;
		//camImages = NULL;
    };

	float imageScale;
	float maxICPError;
	//float faceSearchRegionRatio;

	//b3di::CameraParams* inputCams;
	//b3di::ImageContainer* camImages;
	std::vector<b3di::CameraParams> inputCams;
	std::vector<b3di::FrameStorePtr> frameStores;

	cv::Ptr<HeadPoseIntData> headPoseData;

	int numThreads;			// number of threads for computing depth in parallel (if supported)
    bool computeHeadDepth;

	b3di::DepthCamType depthCameraType;
	//b3di::DepthProcessor::DepthProcessorConfig depthProcConfig;

	// Optional: face landmarks of the first M cam frame
	// If not provided, the landmarks will be detected in the first trackHeadPoseFrame
	b3di::FaceTracker::FaceLandmarks faceLandmarks;

	cv::String asmTrackerDataPath;		// stasm face tracker training data path. If empty, default paths will be searched


    int wait;
    bool show;
    bool verbose;
};


struct TrackHeadPose_Output {

	TrackHeadPose_Output() {};

	std::shared_ptr<b3di::CameraContainer> headPoseContainerPtr;		// head pose for each frame 
	//b3di::ImageContainer headDepthContainer;		// all computed head depth images in L cam space
	//std::vector<unsigned long> frameTimes;		// timestamps for each frame in the container
	b3di::FaceTracker::FaceLandmarks faceLandmarks;		// face landmarks of the first frame in camImages[2]

	b3di::FrameStorePtr depthFramesPtr;		// output depth frames if comuteHeadDepth is true

//	cv::Mat headDepthImg;		// last head depth image. Redundant. Removed.
};

int trackHeadPose(TrackHeadPose_Input& input, TrackHeadPose_Output& output);

int initHeadPoseTracker(TrackHeadPose_Input& input, TrackHeadPose_Output& output);

int trackHeadPoseFrame(unsigned long frameTime, const cv::Mat camImages[], trimesh::xform& headXf, float& trackingError,
    TrackHeadPose_Input& input, TrackHeadPose_Output& output);

// Call this in a loop for all frames
//int trackHeadPoseFrame(int frameIndex, TrackHeadPose_Input& input, TrackHeadPose_Output& output);