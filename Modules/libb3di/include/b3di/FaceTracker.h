
#pragma once


#include <vector>
#include <opencv2/core.hpp>

#include "B3d_defs.h"
#include "CameraParams.h"
#include "PolyMesh.h"

namespace b3di {

// This is defined for a portrait camera with the face centered horizontally at the upper half of the image
static const float FACE_SEARCH_WIDTH = 0.8f;
static const float FACE_SEARCH_TOP_BORDER = 0.1f;
static const float FACE_SEARCH_BOTTOM_BORDER = 0.1f;

// Base class for all face tracker classes
// You should not be instantiating an object from this class directly
class FaceTracker {
public:

	static const int LM_COUNT_ASM = 77;
	//static const int LM_COUNT_MT_1 = 83;
	//static const int LM_COUNT_MT_2 = 171;

	enum FaceTrackerType {
		FT_UNKNOWN,
		FT_ASM,			// ASM face tracker
		FT_EXT,			// Extended face landmarks to ASM face landmarks
		//FT_MT,		// Meitu face tracker
        FT_FPP,			// Face++ face tracker
	};

	enum LandmarkCvtType {
		LM_MT171_MT83	// Convert MT 171 to MT 83 points
	};

	// Left is the left one when looking at the face (not from the person's viewpoint)
	enum LandmarkKey {
		// these work for getLandmarkIndex
		LK_PUPIL_L,
		LK_PUPIL_R,
		LK_NOSE_TIP,
		LK_NOSE_BOTTOM,
		LK_NOSE_TOP,
		LK_NOSE_LEFT,
		LK_NOSE_RIGHT,
		LK_NOSE_BOTTOM_LEFT,
		LK_NOSE_BOTTOM_RIGHT,
		LK_FACE_TOP,
        LK_FACE_TOP_LEFT,
        LK_FACE_TOP_RIGHT,
		LK_FACE_BOTTOM,
        LK_FACE_BOTTOM_LOWER_LEFT,
        LK_FACE_BOTTOM_LOWER_RIGHT,
        LK_FACE_BOTTOM_UPPER_LEFT,
        LK_FACE_BOTTOM_UPPER_RIGHT,
        LK_LIPS_CENTER_TOP,
        LK_LIPS_CENTER_BOTTOM,
        LK_UPPER_LIP_CENTER_BOTTOM,
        LK_LOWER_LIP_CENTER_TOP,
		LK_LEFT_EYEBROW_TOP,
		LK_RIGHT_EYEBROW_TOP,
        LK_LEFT_EYEBROW_OUTSIDE,
        LK_RIGHT_EYEBROW_OUTSIDE,
		LK_LEFTMOST_EYE_POINT,
		LK_RIGHTMOST_EYE_POINT,
		LK_CONTOUR_1,			// face contour landmarks first point
		LK_CONTOUR_2,			// face contour landmarks last point
								// add more landmark keys here

		// these work for getLandmarkIndices
		LK_LEFT_EYE,
		LK_RIGHT_EYE,
		LK_UPPER_LEFT_EYE,
		LK_LOWER_LEFT_EYE,
		LK_UPPER_RIGHT_EYE,
		LK_LOWER_RIGHT_EYE,
		LK_LEFT_EYEBROW,
		LK_RIGHT_EYEBROW,
		LK_UPPER_LEFT_EYEBROW,
		LK_UPPER_RIGHT_EYEBROW,
		LK_LOWER_LEFT_EYEBROW,
		LK_LOWER_RIGHT_EYEBROW,
		LK_UPPER_LIPS,
		LK_LOWER_LIPS,
		LK_INNER_UPPER_LIPS,
		LK_INNER_LOWER_LIPS,
		LK_NOSE_CONTOUR,
        LK_NOSE_BOTTOM_CONTOUR,      // bottom of the nose
		LK_INNER_MOUTH,				// mouth opening
		LK_OUTER_MOUTH,				// mouth including lips
		LK_OUTER_CONTOUR,
		LK_LEFT_EYEBROW_OTHER_TOP,
		LK_RIGHT_EYEBROW_OTHER_TOP
	};

	// 3d object type for converting 3d landmarks to objects
	enum LandmarkObjType {
		LO_POINT,			// convert each landmark to a point
		LO_PYRAMID,			// convert each landmark to a 4 sided pyramid (4 vertices and 4 triangles)
	};

	struct FaceLandmarks {

		FaceLandmarks() { trackerType = FT_UNKNOWN; coordUnit = 1.0; }
		FaceLandmarks(const FaceLandmarks& a) { *this = a; }
		bool empty() const { return landmarks2d.size() == 0; }
		FaceLandmarks& operator= (const FaceLandmarks& a) {
			trackerType = a.trackerType;
			landmarks2d = a.landmarks2d;
			landmarks3d = a.landmarks3d;
			imageSize = a.imageSize;
			coordUnit = a.coordUnit;
			return *this;
		}

		FaceTrackerType trackerType;
		std::vector<cv::Point2f> landmarks2d;
		std::vector<cv::Point3f> landmarks3d;
		cv::Size imageSize;		// image size for the 2d landmarks
		float coordUnit;		// Coordinate unit of the 3d landmarks. 1 is for 1mm and 10 is for 1cm.

	};

    virtual ~FaceTracker();

	// These must be overriden by the dervied classes
	// Find 2d face landmarks in an image
	// landmarkCount is the number of landmarks to track. Set 0 to use default. The supported numbers depend on the tracker type
	// Set startFromOldlanmarks to start tracking from existing landmarks, if they are present. The support of this depends on the tracker type
	// searchRect controls a sub region to search in image. if searchRect is empty, it will search the entire image
	// if refinePupils is true, perform additional processing to find more accurate puipl centers
	virtual bool findLandmarks(const cv::Mat& image, int landmarkCount, bool startFromOldLandmarks, const cv::Rect& searchRect,
		bool refinePupils=true) = 0;
	// widthRatio, topBorder and bottomBorder control the search region as a ratio of the image width and height.
	virtual bool findLandmarks(const cv::Mat& image, int landmarkCount = 0, bool startFromOldLandmarks = false,
		float widthRatio = FACE_SEARCH_WIDTH, float topBorder = FACE_SEARCH_TOP_BORDER, float bottomBorder = FACE_SEARCH_BOTTOM_BORDER);

	virtual FaceTrackerType getTrackerType() const = 0;
	// Get a single landmark index from a landmark key
	virtual int getLandmarkIndex(LandmarkKey key) const = 0;
	// return multiple indices for a landmark key
	virtual void getLandmarkIndices(LandmarkKey key, std::vector<int>& landmarkIds) const = 0;
	// Find head pose (x, z, z) rotation angles in radians from 3d landmarks
	virtual bool getHeadPose(cv::Vec3d& headPose) const = 0;

	// Find head pose (x, z, z) rotation angles in radians from 2d landmarks, depth image and depth cam
	// use this function if 3d landmarks are not available or not reliable
	virtual bool getHeadPose(const cv::Mat& depthImage, const CameraParams& depthCam, cv::Vec3d& headPose) const = 0;

	// Get the outer face contour 2d landmark indices
	virtual void getFaceContourIndices(std::vector<int>& contourIds) const = 0;

	// return true if the point is a good feature point to use for tracking
	// this normally excludes outer landmarks or non-feature based landmarks
	virtual bool isGoodPointToTrack(int landmarkId) const = 0;

	// not implemented
	virtual bool isEyeClosed() const = 0;
	// write a file to a directory
	virtual bool readFile(const cv::String& inputFile);
	virtual bool writeFile(const cv::String& outputFile);

	// Get bounding rect for the face
	// findLandmarks must be called first
	virtual cv::Rect getFaceRect() const;

	// get the 2d center point of the face
	virtual cv::Point2f getFaceCenter2d() const;
	virtual cv::Point3f getFaceCenter3d() const;

	// get the center of the nose 2d landmarks
	virtual cv::Point2f getNoseCenter2d() const;

    // Get the bounding rect of a face feature identified by key. Key must be one of the landmark group keys compatible with getLandmarkIndices
    virtual cv::Rect getFaceFeatureRect(LandmarkKey key) const;

	// Draw landmarks to the image.
	virtual void drawLandmarks2d(cv::Mat& image, cv::Scalar color, int thickness, bool connectPoints = true) const;

    // Draw landmark feature with the color value in image
    // featureKey must be one that forms a closed polygon
    // if lineThickness <0, fill the interior of the feature
    virtual void drawLandmarkFeature(cv::Mat& image, FaceTracker::LandmarkKey featureKey, cv::Scalar color,
        int lineThickness = -1) const;

    // Fill the inside of face landmarks with fillColor on image
	// NOT TESTED
	virtual void fillFaceMask(cv::Mat& image, cv::Scalar fillColor=255);

	// Get 3d landmarks from 2d landmarks, depth image, and camera intrinsic parameters
	virtual void get3dLandmarksFrom2d(const cv::Mat& depthImage, const CameraParams& camParams);

	// Filter landmark points to ensure that the points on the same feature curve do not wrap back (as a result of points reprojected from another view)
	// and the points do not extend outside of the outer face contour (or faceMask if provided)
	virtual void filterLandmarks2d(const cv::Mat& faceMask = cv::Mat());

	// Refine the outer contour landmarks by scaling the landmarks Y coordinates match to chin depth boundary in depthImage
	// the depth image is assumed to be cyl depth map. it may not work for perspective depth map
	// maxHeadRect controls the max bounds for the search of the chin depth boundary. If empty, search the entire image
	// depthUnitScale is the depth to Z unit scale
	//virtual void refineLandmarks2dByDepth(const cv::Mat& depthImage, cv::Rect maxHeadRect=cv::Rect(), double depthScale=0.01);
	// if smoothDepth is true, the depth image is already smoothed (no holes). Otherwise, the depthImage is unsmoothed and contains holes
	virtual void refineLandmarks2dByDepth(const cv::Mat& depthImage, const CameraParams& camParams, 
		bool smoothDepth=true, cv::Rect headRect=cv::Rect() ) {};

	// make the outer contour symmetrical relative to the center
	virtual void makeSymmetrical() {};

	// Return a PolyMesht that contains a 3d object for each landmark
	// objSize is the size of the object in mm (N/A for LO_POINT)
	void get3dLandmarkObjects(b3di::PolyMesh& mesh, float objSize = 3, LandmarkObjType objType = LO_PYRAMID) const;

	const FaceLandmarks& getFaceLandmarks() const { return _faceLandmarks; }
	const std::vector<cv::Point2f>& getLandmarks2d() const { return _faceLandmarks.landmarks2d; }
	const std::vector<cv::Point2f>& getLandmarks() const { return _faceLandmarks.landmarks2d; }
	const std::vector<cv::Point3f>& getLandmarks3d() const { return _faceLandmarks.landmarks3d; }
	void setFaceLandmarks(const FaceLandmarks& landmarks) {
		_faceLandmarks = landmarks;
	}

	// Convert landmarks. Only MT 171 to 83 points is supported for now
	bool convertLandmarks(LandmarkCvtType cvtType = LM_MT171_MT83);

	// Transform and project landmarks to a different view
	// cam should have the intrinsic params of the new view
	void projectLandmarks(const trimesh::xform& transform, const CameraParams& cam);

	float getFaceDistance() { return getFaceDistance(_faceLandmarks.landmarks3d); }

	// Scale the 2d landmarks to match the dstSize. If dstSize is Size(), use scaleX and scaleY instead.
	void scaleLandmarks2d(cv::Size dstSize, double scaleX = 0, double scaleY = 0);
	// Translate the 2d landmarks to a space centered at orginPt
	void translateLandmarks2d(cv::Point2f originPt);

	// Return landmark keys to face features (eyes, lips, eye brows etc.)
	static std::vector<LandmarkKey> getFaceFeatureKeys();

	// These are static class functions and can be called without a FaceTrcker object
	// Use this to read a landmark file of unknown type
	// Return the landmarks and the tracker type (based on the number of landmarks in the file)
	static bool readFile(const cv::String& inputFile, FaceLandmarks& landmarks);
	// Write landmarks and optional extended landmarks to file
	static bool writeFile(const cv::String& outputFile, const FaceLandmarks& landmarks, 
		const FaceLandmarks& extendedLandmarks =FaceLandmarks());

	static bool read(const cv::FileStorage& fs, FaceLandmarks& landmarks);
	static bool write(cv::FileStorage& outputFile, const FaceLandmarks& landmarks,
		const FaceLandmarks& extendedLandmarks = FaceLandmarks());

	// Ge the bounding rect from the 2d landmarks. Set excludeOuter to true to not consider the outer contour landmarks
	// use cropRect to not include any landmark points outside of the cropRect
	static cv::Rect getFaceRect(const std::vector<cv::Point2f>& landmarks, bool excludeOuter = false, cv::Rect cropRect=cv::Rect());
	// Get an 8-bit face mask image from a face rect
	static cv::Mat getFaceMask(const cv::Size& imageSize, const cv::Rect& faceRect, int margin, int featherWidth);

    // Return a 8-bit mask for the feature given in key.
    // The key must be one of LK_INNER_MOUTH, LK_LEFT_EYE, LK_RIGHT_EYE, LK_OUTER_CONTOUR
    // Pixels inside the feature will be 0 and outside will be 255
    // If inverted is true, the pixel values will be inverted (255 inside and 0 outside)
    bool getFeatureMask(LandmarkKey key, cv::Mat& featureMask, bool inverted = false, cv::Rect maskRect=cv::Rect()) const;

    // Get 3d landmarks from 2d landmarks, depth image, and cylindrical camera parameters
    // Only works for cylindrical map, assumes landmarks2d always corresponds to a pixel location with valid depth values
    static bool get3dLandmarksFromCyl2d(const std::vector<cv::Point2f>& landmarks2d, const cv::Size& imageSize,
        const cv::Mat& depthImage, const CameraParams& camParams, std::vector<cv::Point3f>& landmarks3d);

    // FIXME: this function never return valid "landmarks3d", because internally the FaceTracker is constructed with UNKNOWN type
	// Get 3d landmarks from 2d landmarks, depth image, and camera intrinsic parameters
	static void get3dLandmarksFrom2d(const cv::Mat& depthImage, const std::vector<cv::Point2f>& landmarks, const CameraParams& camParams, std::vector<cv::Point3f>& landmarks3d);

    // Project 3d landmarks to target cylindrical map space
    // cylCam must a cylindrical projeciton camera
    // cylMapSize is the target cylindrical map size
    static bool project3dLandmarksToCyl2d(const std::vector<cv::Point3f>& landmarks3d, const CameraParams& cylCam, cv::Size cylMapSize, std::vector<cv::Point2f>& landmarks2d);

	static void get3dLandmarkObjects(const std::vector<cv::Point3f>& landmarks3d, b3di::PolyMesh& mesh, float objSize = 3, LandmarkObjType objType = LO_PYRAMID);

	//static void refineLandmarks2dByDepth(std::vector<cv::Point2f>& landmarks2d, const cv::Mat& depthImage, cv::Rect maxHeadRect = cv::Rect(), double depthScale = 0.01);

	// Get left/right eye rects from landmarks
	static bool getEyeRects(const FaceLandmarks& faceLandmarks, cv::Rect eyeRects[2]);

	// Get the average face distance from the 3d landmarks
	static float getFaceDistance(const std::vector<cv::Point3f>& landmarks3d);

	// return true if landmarkIndex is an exterior landmark
	// landmarkCount is the number of landmarks
	static bool isOuterLandmark(int landmarkCount, int landmarkIndex);

	//static void scaleLandmarks2d(const std::vector<cv::Point2f>& srcLandmarks, std::vector<cv::Point2f>& dstLandmarks, float scaleX, float scaleY);
	// Scale the 2d landmarks to match the dstSize. If dstSize is Size(), use scaleX and scaleY instead.
	static void scaleLandmarks2d(const FaceLandmarks& srcLandmarks, FaceLandmarks& dstLandmarks, cv::Size dstSize, double scaleX=0, double scaleY=0);
	// Translate the 2d landmarks to a space centered at orginPt
	static void translateLandmarks2d(const FaceLandmarks& srcLandmarks, FaceLandmarks& dstLandmarks, cv::Point2f originPt);

	// Return cv::Ptr. Use these instead of newFaceTracker to avoid having to deallocate the pointers
	static cv::Ptr<FaceTracker> newFaceTrackerPtr(const FaceLandmarks& landmarks) { return cv::Ptr<FaceTracker>(newFaceTracker(landmarks)); }
	static cv::Ptr<FaceTracker> newFaceTrackerPtr(const cv::String& inputFile) { return cv::Ptr<FaceTracker>(newFaceTracker(inputFile)); }
	static cv::Ptr<FaceTracker> newFaceTrackerPtr(FaceTrackerType trackerType);

	// Convert fromLandmarks to toLandmarks using cvtType
	static bool convertLandmarks(const FaceLandmarks& fromLandmarks, FaceLandmarks& toLandmarks, LandmarkCvtType cvtType=LM_MT171_MT83);

	// Move any 2d landmarks outside the mask image of a face horizontally until they they are just inside the depth image
	// The faceMask can be created from the face contour or a depth image
	static void moveLandmarks2dInsideMask(std::vector<cv::Point2f>& landmarks2d, const cv::Mat& faceMask, const int maxSearch = 100);

	// get a 2x zoom rect to the face
	static cv::Rect getFaceZoomRect(cv::Size imageSize, const std::vector<cv::Point2f>& faceLandmarks);

	// Find a face in an image and return its bounding box if found, or an empty rect if not.
	// If xMargin or yMargin is not zero, exclude border pixels defined in xMargin and yMargin from the search
	static cv::Rect findFaceRect(const cv::Mat& image, int xMargin=0, int yMargin=0);

protected:

    // These should not be used externally. They are used by newFaceTrackerPtr
    // return a new FaceTracker pointer from landmarks or a file
    // The pointer could be of any of FaceTracker types
    // The pointer should be deallocated by the caller!!!
    static FaceTracker* newFaceTracker(const FaceLandmarks& landmarks);
    static FaceTracker* newFaceTracker(const cv::String& inputFile);

	void filterLandmarksZ();

	// Make the constructer protected so you can't instantiate an object of this class
	FaceTracker();

	FaceLandmarks _faceLandmarks;


};

} // namespace b3di
