#pragma once

#include <vector>
#include <opencv2/core.hpp>

#include "FaceTracker.h"

namespace b3di {

// STASM Face Tracker
class FaceTrackerASM : public FaceTracker {
public:

	FaceTrackerASM();
	virtual ~FaceTrackerASM();

	static bool initTracker(const cv::String& trainingDataPath);

	virtual FaceTrackerType getTrackerType() const {
		return FT_ASM;
	}

	// These must be overriden by the dervied classes
	// Find face landmarks in an image; if multiple faces are found, only the largest face will be used
	// image will be automatically converted to gray scale image if it is not
	// LandmarkCount is ignored. Only 77 landmarks is supported right now.
	// if startFromOldLandmarks is set, the search will start with old landmarks if they are present
	// The param may not be supported by all methods
	virtual bool findLandmarks(const cv::Mat& image, int landmarkCount = 0, bool startFromOldLandmarks = false, 
		const cv::Rect& searchRect = cv::Rect(), bool refinePupils=true);


	virtual bool readFile(const cv::String& inputFile);

	// Get landmark index from a landmark key
	virtual int getLandmarkIndex(LandmarkKey key) const;
	virtual void getLandmarkIndices(LandmarkKey key, std::vector<int>& landmarkIds) const;

	virtual bool getHeadPose(cv::Vec3d& headPose) const;

	// Find head pose (x, z, z) rotation angles in radians from 2d landmarks, depth image and depthCam
	// use this function if 3d landmarks are not available
	virtual bool getHeadPose(const cv::Mat& depthImage, const CameraParams& depthCam, cv::Vec3d& headPose) const;

	// Get the outer face contour 2d landmark indices
	virtual void getFaceContourIndices(std::vector<int>& contourIds) const;

	virtual bool isGoodPointToTrack(int landmarkId) const;

	virtual bool isEyeClosed() const;

	// make the outer contour symmetrical relative to the center
	virtual void makeSymmetrical();

	// Refine the chin landmarks Y position based the depth map
	// the depth image is assumed to be cyl depth map. it may not work for perspective depth map
	// camParams is the cylindrical camera parameters
	// if smoothDepth is true, the depth image is already smoothed (no holes). Otherwise, the depthImage is unsmoothed and contains holes
	// if headRect is not empty, limit the refinement to headRect
	virtual void refineLandmarks2dByDepth(const cv::Mat& depthImage, const CameraParams& camParams, 
		bool smoothDepth = true, cv::Rect headRect=cv::Rect());

protected:

	static bool _stasmInitOk;

};

} // namespace b3di