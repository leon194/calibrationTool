
#pragma once

#include <vector>

#include <opencv2/core.hpp>

#include "B3d_defs.h"
#include "CameraContainer.h"
#include "ImageContainer.h"
//#include "DepthProcessor.h"
#include "FrameStore.h"
#include "DepthCamProps.h"
#include "B3d_utils.h"
#include "B3d_types.h"

class FacePoseIntData;

/** This Error Code should match the one in HeadScannerEvent.h */
enum TrackFaceStatus {
	HEADSCANNER_READY_TO_SCAN = 0,
	HEADSCANNER_FACE_NOT_FOUND = 1,
	HEADSCANNER_FACE_NOT_FRONTAL = 2,
	HEADSCANNER_FACE_TOO_CLOSE = 3,
	HEADSCANNER_FACE_TOO_FAR = 4,
	HEADSCANNER_FACE_NOT_CENTERED = 5,
	// HEADSCANNER_ERROR_INVALID_INPUT = 6  // e.g. camImages empty, cannot compute depth, cannot find calibration files
};

struct TrackFace_Input {

	TrackFace_Input() {

		wait = 0;
		show = true;
		verbose = true;

		fastTracking = true;		// default is true

		disableFaceDetection = false;	// default is false

//		imageScale = 0.25f;		// not used
//		faceSearchRegionRatio = (float)b3di::DEPTHCAM_PROPS[b3di::DEPTHCAM_B3D2].faceDetectionWindowWidthRatio;
//		faceSearchRegionRatio = (float)b3di::PORTRAIT_CAM_FACE_REGION;
		//		maxICPError = (float)b3di::ICP_MAX_ERROR;

		//inputCams = NULL;
		//camImages = NULL;

		//		rot_thresh    = cv::Vec3d(1.00, 0.15, 0.10);
		//rot_thresh = cv::Vec3d(0.78, 0.18, 0.18);			// Limit X rotation to about 45 degrees

		rot_thresh = cv::Vec3d(b3di::degreeToRad(25.0), b3di::degreeToRad(15.0), b3di::degreeToRad(15.0));
		//dist_thresh[0] = b3di::DEPTHCAM_PROPS[b3di::DEPTHCAM_B3D2].minScanDistance;
		//dist_thresh[1] = b3di::DEPTHCAM_PROPS[b3di::DEPTHCAM_B3D2].maxScanDistance;
															//		dist_thresh   = cv::Vec2d(300, 450);
		//dist_thresh = cv::Vec2d(b3di::TRACK_FACE_NEAR_RANGE, b3di::TRACK_FACE_FAR_RANGE);		// default
		//		dist_thresh = cv::Vec2d(250, 760);		// increase range for testing only
		offset_thresh = cv::Vec2d(0.1f, 0.1f);		// ignored 

		frameIndex = -1;
		trackingCamId = 2;
		depthCameraType = b3di::DEPTHCAM_B3D2;
		//depthProcConfig = b3di::DepthProcessor::DepthProcessorConfig();

		scanMode = b3di::SCAN_FULL_HEAD;

	};

	cv::Vec3d rot_thresh;     // rotation angles in radians
	//cv::Vec2d dist_thresh;	  // face distance range in mm
	cv::Vec2d offset_thresh;  // Max face rect offset from target in fraction of image width and height

//	float imageScale;			// ignored
	//	float maxICPError;
	//float faceSearchRegionRatio;		// center region size ratio for the face search 1.0 will search the entier image

	//b3di::CameraParams* inputCams;	// 3 or 4 cameras (L/R/M/C) where C is only needed if trackingCamId is 3
	//b3di::ImageContainer* camImages;		// same as the above
	std::vector<b3di::CameraParams> inputCams;
	std::vector<b3di::FrameStorePtr> frameStores;		// set this to use trackFace; not needed if trackFacePoseFrame is called

	// should be 2 for now (M cam)
	int trackingCamId;

	cv::Ptr<FacePoseIntData> facePoseData;
	bool computeHeadDepth;

	cv::String asmTrackerDataPath;		// stasm face tracker training data path. If empty, default paths will be searched

	cv::Rect startFaceRect;   // obtained from MC extrinsic computation
	cv::Rect targetFaceRect;  // obtained from MC extrinsic computation
	int frameIndex;  // tmp solution
	bool faceTrackFailed;
	bool drawFaceRectMcam;
	// vector<Point3f> facePts_M;  // tmp solution, for getting 'targetFaceRect'

	b3di::DepthCamType depthCameraType;
	//b3di::DepthProcessor::DepthProcessorConfig depthProcConfig;

	// Set fast tracking mode (no landmark and depth computation)
	// Face rotation will not be computed and face distance is approximated
	bool fastTracking;	

	// Set to true to disable face detection. 
	// trackFacePoseFrame will always return HEADSCANNER_READY_TO_SCAN and make the function a no-op
	// Default is false
	bool disableFaceDetection;

	b3di::ScanMode scanMode;

	int wait;
	bool show;
	bool verbose;
};

struct TrackFace_Output {

	TrackFace_Output() {

        currentStatus = HEADSCANNER_FACE_NOT_FOUND;

        readyToCapture = false;  // <- could be included in currentStatus
	};

	TrackFaceStatus currentStatus;

	cv::Mat depthImg;

    bool readyToCapture;
};

// For offline processing of captured data
// Get input frames from frameStores
// If depth frames are provided frameStores[4] should contain depth frames and the depth should be in M cam space
// and frameStores[0] and frameStores[1] should be empty
// frameStores[2] should contain M cam images for face tracking
int trackFace(TrackFace_Input& input, TrackFace_Output& output);


// For live tracking
int initFacePoseTracker(TrackFace_Input& input);

// camImages should be an array with 3 items (camImages[0], camImages[1], camImages[2] should contain L, R, M images)
// or 5 items (camImages[4] should contain depth image in M cam space and camImages[0] and camImages[1] should be empty)
int trackFacePoseFrame(const cv::Mat camImages[], cv::Vec3d& faceRotation, float& faceDistance, cv::Rect& faceRect,
	TrackFace_Input& input, TrackFace_Output& output);


// For Windows APP
// Shouldn't be Exposed Externally
void drawUIComponents(const cv::Mat& faceCamImg, const cv::Rect& faceRect, bool readyToCapture,
	const TrackFace_Input& input);