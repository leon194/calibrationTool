#pragma once

#include <vector>

// b3di
#include "CameraParams.h"

namespace b3di {

class EarTracker {

public:
    EarTracker();
    virtual ~EarTracker();

	// number of ear landmarks
	static const int LM_COUNT = 32;

    // A data structure that holds EarLandmarks data
    // currently only used in b3di, not by b3d
    enum EarTrackerType {
        ET_32,  // 16 landmarks for each ear
    };

    struct EarLandmarks {
        EarLandmarks() {
            trackerType = ET_32;
            coordUnit = 1.0f;
        }

        bool empty() const { return landmarks2d.size() == 0 || landmarks3d.size()==0; }

		// invalidate left and/or right landmarks with zeros if either is true
		void invalidate(bool left=true, bool right= true);

		// return true if left and right ear landmarks are valid
		// also return the validity of each ear
		bool valid(bool& hasLeft, bool& hasRight) const;
		bool valid() const { bool l, r;  return valid(l, r); }

        bool writeEarLandmarks(
            const cv::String& outputDir, const cv::String& landmarkFileName, const cv::String& objFileName) const;

        bool readEarLandmarks(
            const cv::String& inputDir, const cv::String& landmarkFileName);

        EarTrackerType trackerType;

        std::vector<cv::Point2f> landmarks2d;
        std::vector<cv::Point3f> landmarks3d;
        cv::Size imageSize;		// image size for the 2d landmarks
        
        float coordUnit;  // Coordinate unit of the 3d landmarks. 1 is for 1mm and 10 is for 1cm.
    };


	// initialize ear detector classifier files
	// if initLandmark is true, also load ear landmark detector model file (~70MB)
    // shall be called only once for EarTracker to work
    static bool initialize(const std::string& modelFilePath, bool initLandmark=true);

    // Call this function to release ~70MB heap memory of the model file
    // After this call, EarTracker cannot function unless it's initialized again
    // should not need to call this function, unless memory usage is a big concern
    static void dispose();

    // read ear landmarks file
    static bool readLandmarksFile(const cv::FileStorage& fs, EarLandmarks& landmarks);

    // write ear landmarks file
    static bool writeLandmarksFile(cv::FileStorage& fs, const EarLandmarks& landmarks);

    // This function is used internally, as we would like to put EarLandmarks info into a struct
    // the output is EarLandmarks struct instead of plain vectors of points
    //
    // Input: textureImage, is in cylindrical space, with face in the center, ears on both sides
    // Requires faceLandmarks to speed up the location of ear bounding boxes
	// if earRects is not null, it should be a pointer to Rect[2] and will return the detected left/right ear rects
	// if hasEars is not full, it should be a pointer to bool[2] and will return ear rect detection status
    static bool detectLandmarks(const cv::Mat& textureImage,  
		const std::vector<cv::Point2f>& faceLandmarks2d,
        const cv::Mat& depthImage, const b3di::CameraParams& depthCam, float coordUnit, bool objSpace,
        EarLandmarks& earLandmarks, cv::Rect* earRects=NULL, bool* hasEars=NULL);

  //  // This function is used by HeadMesh class to obtain ear landmarks in vectors directly
  //  // as currently b3d public API is returning vectors of landmarks, not EarLandmarks struct
  //  //
  //  // Input: textureImage, is in cylindrical space, with face in the center, ears on both sides
  //  // Requires faceLandmarks to speed up the location of ear bounding boxes
  //  static bool detectLandmarks(const cv::Mat& textureImage, const std::vector<cv::Point2f>& faceLandmarks2d,
  //      const cv::Mat& depthImage, const b3di::CameraParams& depthCam, float coordUnit, bool objSpace,
  //      std::vector<cv::Point2f>& earLandmarks2d, std::vector<cv::Point3f>& earLandmarks3d,
		//cv::Rect* earRects = NULL
		//);

    // scale ear landmarks
    //static void scaleLandmarks2d(const std::vector<cv::Point2f>& srcLandmarks, std::vector<cv::Point2f>& dstLandmarks, float scaleX, float scaleY);
    // Scale the 2d landmarks to match the dstSize. If dstSize is Size(), use scaleX and scaleY instead.
    static void scaleLandmarks2d(const EarLandmarks& srcLandmarks, EarLandmarks& dstLandmarks, cv::Size dstSize, double scaleX = 0, double scaleY = 0);

    // get ear rects from earLandmarks
	// earRects should be Rect[2]
    static void getEarRects(const EarLandmarks& srcLandmarks, cv::Rect earRects[]);

    // Get 3d landmarks from 2d landmarks, depth image, and cylindrical camera parameters
    // Only works for cylindrical map, assumes landmarks2d always corresponds to a pixel location with valid depth values
    static bool get3dLandmarksFromCyl2d(const std::vector<cv::Point2f>& landmarks2d, const cv::Size& imageSize,
        const cv::Mat& depthImage, const CameraParams& cylCam, std::vector<cv::Point3f>& landmarks3d);
};

}  // namespace b3di
