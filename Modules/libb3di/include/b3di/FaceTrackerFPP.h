#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

// b3di
#include "CameraParams.h"
#include "FaceTracker.h"

namespace b3di {

// Face++ Face Tracker
class FaceTrackerFPP : public FaceTracker {
public:

    FaceTrackerFPP();
	virtual ~FaceTrackerFPP();

    virtual FaceTrackerType getTrackerType() const {
        return FT_FPP;
    }
    // ----  these must be overridden by the derived classes
    // Find 2d face landmarks in an image
    // landmarkCount is the number of landmarks to track. Set 0 to use default. The supported numbers depend on the tracker type
    // Set startFromOldlanmarks to start tracking from existing landmarks, if they are present. The support of this depends on the tracker type
    // searchRect controls a sub region to search in image. if searchRect is empty, it will search the entire image
    virtual bool findLandmarks(const cv::Mat& image, int landmarkCount, bool startFromOldLandmarks, const cv::Rect& searchRect);

    // Get a single landmark index from a landmark key
    virtual int getLandmarkIndex(LandmarkKey key) const;
    // return multiple indices for a landmark key
    virtual void getLandmarkIndices(LandmarkKey key, std::vector<int>& landmarkIds) const;
    // Find head pose (x, z, z) rotation angles in radians from 3d landmarks
    virtual bool getHeadPose(cv::Vec3d& headPose) const;
    // Get the outer face contour 2d landmark indices
    virtual void getFaceContourIndices(std::vector<int>& contourIds) const;

    // return true if the point is a good feature point to use for tracking
    // this normally excludes outer landmarks or non-feature based landmarks
    virtual bool isGoodPointToTrack(int landmarkId) const;

    // not implemented
    virtual bool isEyeClosed() const;


    static void get3dLandmarksFrom2dFPP(const cv::Mat& depthImage, const std::vector<cv::Point2f>& landmarks,
        const CameraParams& camParams, std::vector<cv::Point3f>& landmarks3d);

    //static bool initModel(const cv::String& modelFilePath);

    static bool findLandmarksFPP(const cv::Mat& image, std::vector<cv::Point2f>& landmarks2d, const cv::String& modelFilePath);
    
    // Write landmarks to outputDir
    // landmarkFileName should end with .yml
    // The 2d landmarks should match the texture map coordinate space
    // The 3d landmarks should be points in the world space where if is relative to the L camera (origin)
    // if objSpace param is true, the 3d landmarks will be transformed to match the head's OBJ file space, where the origin is the center of the head
    // In addition, if objSpace param is true and an objFileName is provided, an OBJ file constructed from the 3d landmark points will be written out
    static bool writeLandmarks(
        const std::vector<cv::Point2f>& landmarks2d, const std::vector<cv::Point3f>& landmarks3d,
        const b3di::CameraParams& headCam, const cv::Mat& textureimg,
        const cv::String& outputDir, const cv::String& landmarkFileName,
        bool objSpace = false, const cv::String& objFileName = "");


private:
    // FacePP tracker related
    //bool _isModelInitialized;
    //MG_FPP_APIHANDLE _handle;

    //std::vector<cv::Point2f> _landmarks2d;
    //std::vector<cv::Point3f> _landmarks3d;
};

} // end of namespace b3di