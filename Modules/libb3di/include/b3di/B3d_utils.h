#pragma once

#include <stdio.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "Vec.h"
#include "XForm.h"
#include "CameraParams.h"
#include "B3d_defs.h"
#include "DepthCamProps.h"

namespace b3di {

/*
* Return true if the expiration date provided is reached
* exp_yyyy (e.g. 2017), 0 for no expiration (always return false)
* exp_mm: month of the year (0-11)
* exp_dd: day of month (1-31)
* Also returns expDate string
*/
bool isExpired(int exp_yyyy, int exp_mm, int exp_dd, cv::String& expDate);

bool isAuthorized(DepthCamType depthCamType, const cv::String deviceID, const CameraParams cameraParams[],
    int exp_yyyy, int exp_mm, int exp_dd);

// Get current time (e.g. 2017.02.08.13.50.23) as a string
// used for creating session folder
cv::String myCurrentTime();

// Platform Dependent
// Use sleepFor
//void mySleep(unsigned milliseconds);

// "to_string" not supported on Android NDK
// Implementation comes from: http://stackoverflow.com/questions/22774009/android-ndk-stdto-string-support
template <typename T>
cv::String to_string(T value)
{
    std::ostringstream os;
    os << value;
    return os.str();
}

// Reture the current time in millisecond
double now_ms(void);

//// Convert between 16-bit depthValue and z values in mm
//// z = depth * depthUnit
//// depthUnit is in mm. Default is DEPTH_UNIT defined in B3d_defs.h
//void setDepthUnit(double depthUnit);
//double getDepthUnit();
//double depthToZ(ushort depthVal);
//ushort zToDepth(double zVal);


double DEPTH_TO_GRAY_SCALE();

void drawHLines(cv::Mat& img, int lineSpace, cv::Scalar lineColor);

void drawText(cv::Mat& showImg, cv::String text, cv::Scalar textColor, int posX, int posY);

// Draw only the 2D points
void draw2DPoints(cv::Mat& image, const std::vector<cv::Point2i> &list_points, cv::Scalar color, int radius = 3, int thickness = 1, int line_type = CV_AA);
void draw2DPoints(cv::Mat& image, const std::vector<cv::Point2f> &list_points, cv::Scalar color, int radius = 3, int thickness = 1, int line_type = CV_AA);

// Draw an arrow into the image
void drawArrow(cv::Mat& image, cv::Point2i p, cv::Point2i q, cv::Scalar color, int arrowMagnitude = 9, int thickness = 1, int line_type = 8, int shift = 0);

// Draw the coordinate axes
void drawAxes2d(cv::Mat& image, const std::vector<cv::Point2i> &list_points2d, int thickness = 2);
void drawAxes2df(cv::Mat& image, const std::vector<cv::Point2f> &list_points2d, int thickness = 2);

// Draw the 3D coordinate axes
void drawAxes3d(cv::Mat& img, const cv::Point3f& origin, float axisLen, const trimesh::xform& transform, const b3di::CameraParams& camParams, int thickness = 2);
void drawAxes3d(cv::Mat& img, const cv::Point3f& origin, float axisLen, const cv::Mat& rot_M, const b3di::CameraParams& camParams, int thickness = 2);

void drawRect(cv::Mat& img, const cv::Rect& r, cv::Scalar color, int thickness = 1);
void draw2DLines(cv::Mat& img, const std::vector<cv::Point2i> &list_points, cv::Scalar color, int thickness = 1, bool closeEnds = true);
void draw2DLines(cv::Mat& img, const std::vector<cv::Point2f> &list_points, cv::Scalar color, int thickness = 1, bool closeEnds = true);
void drawPoly(cv::Mat& img, const std::vector<cv::Point2i> &list_points, cv::Scalar color);

// Computes the norm of the translation error
double get_translation_error(const cv::Mat &t_true, const cv::Mat &t);

// Computes the norm of the rotation error
double get_rotation_error(const cv::Mat &R_true, const cv::Mat &R);

// Exapand a rect by xMaring and yMarin in width and height by keeping the center stationarily
cv::Rect growRect(const cv::Rect& oldRect, int xMargin, int yMargin);
cv::Rect scaleRect(const cv::Rect& oldRect, double xScale, double yScale=0);

cv::Mat xformToMat3x3(const trimesh::xform& xf);
trimesh::xform mat3x3ToXform(const cv::Mat& mat);
cv::Mat xformToMat4x4(const trimesh::xform& xf);
trimesh::xform mat4x4ToXform(const cv::Mat& mat);

//void xformPoints3df(std::vector<Point3f>& points3d, const xform& xf, float maxZ = 0);
//void xformPoints3d(std::vector<Vec3d>& points3d, const xform& xf, float maxZ = 0);
void xformPoints3df(std::vector<cv::Point3f>& points3d, const trimesh::xform& xf);
void xformPoints3d(std::vector<cv::Vec3d>& points3d, const trimesh::xform& xf);

void xformPoints3df(std::vector<cv::Vec3f>& points3d, const trimesh::xform& xf/*, float maxZ*/);

inline cv::Point3f xformPoint(const cv::Point3f& pt, const trimesh::xform& xf) {
	trimesh::vec3 ipt = xf*trimesh::vec3(pt.x, pt.y, pt.z);
	return cv::Point3f(ipt[0], ipt[1], ipt[2]);
}

cv::Point3f getCentroid(const std::vector<cv::Point3f>& points, const cv::Rect2f& boundRect= cv::Rect2f());

// Transform camera space 3d points in fromCam space to toCam space and get the projected image space points
void xformAndProjectPoints(const std::vector<cv::Point3f>& fromPoints, const b3di::CameraParams& fromCam, const b3di::CameraParams& toCam, std::vector<cv::Point2f>& toPoints);

cv::Rect getBoundRect(const std::vector<cv::Point2f>& points);
cv::Rect getBoundRect(const std::vector<cv::Point>& points);

template<class T>
void translatePoints(std::vector<T>& points, const T& t) {
	for (auto& p : points) {
		p += t;
	}
}
//void translatePoints(std::vector<cv::Point>& points, int x, int y);

template<class T>
void scalePoints(std::vector<T>& points, double s) {
	for (auto& p : points) {
		p *= s;
	}
}

template<class T>
inline bool PointInRect(const T& pt, const cv::Rect& r) {
	return pt.x >= r.x && pt.x <= (r.x + r.width - 1) && pt.y >= r.y && pt.y < (r.y + r.height - 1);
}

template<class T>
T getCenterPoint(const std::vector<T>& pts) {
	T center;
	for (const auto& p : pts) {
		center += p;
	}
	if (pts.size()>0)
		center /= (int)pts.size();
	return center;
}

// compute the centroid of elements in vec
// T should match the item type of vec
// vec must be continuous
template<class T>
void computeCentroid(const cv::Mat& vec, T& centroid) {
	centroid = T();
	int n = vec.rows*vec.cols;
	const T* ptr = (T*)vec.data;
	for (int i = 0; i < n; i++, ptr++) {
		centroid += *ptr;
	}
	if (vec.cols>0)
		centroid /= vec.cols;
}

// translate items in vec by offset
// T should match the item type of vec
// vec must be continuous
template<class T>
void translateItems(cv::Mat& vec, const T& offset) {
	int n = vec.rows*vec.cols;
	T* ptr = (T*)vec.data;
	for (int i = 0; i < vec.cols; i++, ptr++) {
		*ptr += offset;
	}
}


// get rotation and translation from a 4x4 matrix
// Assume the rotation order is Mx*My*Mz
// Set flipXYZ to true for Mz*My*Mx
void getRotationFromXform(const trimesh::xform& m4x4, double& xrot, double& yrot, double& zrot, bool flipXYZ = true);

void xformToDof6(const trimesh::xform& xf, cv::Vec3d& rotation, cv::Vec3d& trans);
trimesh::xform Dof6ToXform(const cv::Vec3d& rotation, const cv::Vec3d& trans);

// Converts a given Rotation Matrix to Euler angles
cv::Mat rot2euler(const cv::Mat & rotationMatrix);

// Converts a given Euler angles to Rotation Matrix
cv::Mat euler2rot(const cv::Mat & euler);

// convert 16-bit input value to 8-bit rainbow color
// valueDivider is the number the input values are divided by (control the band width of the rainbow)
void convert16ToRainbowColor(const cv::Mat& inputMap, cv::Mat& outColor, int valueDivider = 10);
void overlayImage(const cv::Mat& bottomImage, const cv::Mat& topImage, float topAlpha, cv::Mat& outImage);

//void xyzToDepthMap(const cv::Mat& xyzMap, cv::Size outSize, cv::Mat& depthMap, double depthResolution = 0);

//void disparityToDepth(const cv::Mat& disparityMap, double fx, double baseline, cv::Mat& depthMap, double depthResolution = DEPTH_RESOLUTION);


//void projectPointsToDepth(const std::vector<Point3f>& points3d, Size sourceSize, const Mat& cameraMatrix, const Mat& distCoeffs, Size outSize, cv::Mat& depthMap, cv::Mat& xyMap, double depthResolution = 0.1);
//void projectPointsToDepth(const std::vector<Point3f>& points3d, Size sourceSize, const Mat& cameraMatrix, const Mat& distCoeffs, Size outSize, cv::Mat& depthMap, double depthResolution = DEPTH_RESOLUTION);


// Read/Write mat as yml file
bool saveMatToYML(const cv::Mat& m, const cv::String fileName, const cv::String key);
bool readMatFromYML(const cv::String fileName, cv::Mat& m, const cv::String key);

void blendWithMask(const cv::Mat &srcImage, cv::Mat &targetImage, const cv::Mat &srcAlpha);

// Blend srcImage with targetImge using srcAlpha. If chromaKeyColorPtr is not null, the pixel colors matching chromaKeyColor will be discarded
// if removeHelights is true, exclude pixels much brigter than its corresponding pixels in the other image
void blendWithMaskC3(const cv::Mat &srcImage, cv::Mat &targetImage, const cv::Mat &srcAlpha, 
	const cv::Vec3b* chromaKeyColorPtr=nullptr);
void blendWithMask(const cv::Mat &image1, const cv::Mat &image2, const cv::Mat &mask, cv::Mat &out, int blendMode=0);
// return a mask image where pixels in inputImage that match chromaKey are zero and others are 255
// return the number of pixels that match the chromaKey
int getMaskFromChromaKey(const cv::Mat& inputImage, cv::Vec3b chromaKey, cv::Mat& maskImage);


bool saveFaceFeaturesToFile(const cv::String fileName, const cv::Mat& points2d, const cv::Mat& points3d, const cv::Mat& headRotMat, const cv::Vec3d& poseAngles);
bool readFaceFeaturesFromFile(const cv::String fileName, cv::Mat& points2d, cv::Mat& points3d, cv::Mat& headRotMat, cv::Vec3d& poseAngles);

void erodeMask(const cv::Mat& srcMask, cv::Mat& dstMask, int erodeSize, int holeSize=0);

void erodeMaskBorders(const cv::Mat& srcMask, cv::Mat& dstMask, int borderWidth, int minBorderLength = 1);

// Plot values of an 1d float vector
// Width of the plot is n x binWidth, where n is the size of the vector
// maxVal is the max. value of items in v1d
void plotVec1d(const std::vector<float>& v1d, int plotHeight, int binWidth, float maxVal, cv::Mat& plotImg, uchar lineColor = 255);


void showImage(const cv::String& winTitle, const cv::Mat& img, int maxWinSize = 960);
void showDepthImage(const cv::String& winTitle, const cv::Mat& depthImg, double valScale = b3di::DEPTH_TO_GRAY_SCALE());

// show depth image as a pseudo color image
void showDepthContours(const cv::String& winTitle, const cv::Mat& depthImg, int stepSize=200);

// show depth map overlay on a color image
// both depth and color image should be the same size
void showDepthOnColorImage(const cv::String& winTitle, const cv::Mat& depthMap, const cv::Mat& colorImage, int stepSize=200);

// fit a rect inside a bounding rect
void fitRectToBounds(const cv::Rect& bounds, cv::Rect& rect);
// crop a rect to imageSize
inline void cropRectToSize(const cv::Size& imageSize, cv::Rect& rect) {
    fitRectToBounds(cv::Rect(0, 0, imageSize.width, imageSize.height), rect);
}

// Return a mask of srcImage with holes (lass than holeSize) closed
void getHoleClosedMask(const cv::Mat& srcImage, int holeSize, cv::Mat& dstMask);

cv::String num2str(int num, int width = 0);

inline double degreeToRad(const double x)
{
	return x*double(CV_PI) / double(180);
}

inline float degreeToRad(const float x)
{
	return x*float(CV_PI) / float(180);
}

inline double radToDegree(const double x)
{
	return x*double(180) / double(CV_PI);
}

inline float radToDegree(const float x)
{
	return x*float(180) / float(CV_PI);
}

// Compute face search ratio from the camera's intrinsic params
// Use smaller ratio for wider FOV
float computeFaceSearchRatio(const CameraParams& cam);

void sleepFor(unsigned milliseconds);

// Compute a gradient map (8-bit) from a color or gray scale image (127 means 0 gradient)
// 
//void getGradientMap(const cv::Mat& sourceMap, cv::Mat& gradMap, bool preFilter = true, int kSize = 3, int yOffset=0);

void resizeImage(const cv::Mat& srcImage, cv::Mat& dstImage, cv::Size dstSize, double dstScale = 0);

// if interpolate is true, interpolate the depth values when resizing
void resizeDepthImage(const cv::Mat& srcImage, cv::Mat& dstImage, cv::Size dstSize, double dstScale = 0, bool interpolate=false);


// Find all s in srcSrc and replace with t
cv::String string_replace(const cv::String& srcStr, const cv::String& s, const cv::String& t);

void splitString(const cv::String& str, char splitChar, std::vector<cv::String>& subStrs);

void balanceWhite(cv::Mat& image, double discardRatio = 0.0001, cv::Scalar targetWhite = cv::Scalar::all(255), bool centerWeighted=true);

// In-paint black pixels
void fillBlackPixels(const cv::Mat& srcImage, cv::Mat& dstImage);
void fillBlackPixels2(const cv::Mat &srcImage, cv::Mat &dstImage, cv::Size filterSize);

// Get a curve from a list of points (at least 3 points)
// The curve contains a list of points (with distance less than 1)
// if adjacent is true, force the points to be adjacent to the previous one
bool pointsToCurve(const std::vector<cv::Point2i>& points, std::vector<cv::Point2i>& curve, 
	bool closeLoop = false, bool smoothCurve = false, bool adjacentPoints=false);

// Return true if the machine byte order is Little Endian
bool isLittleEndian();

// Swap byte order of a 32-bit int or float
void swap32(unsigned int *val);


// Replace cv::threshold
void thresholdImage8(const cv::Mat& srcImage, cv::Mat& dstImage, const uchar thresh);

//// Return the number of matching features between 2 images
//int findMatchingFeatures(const cv::Mat& image1, const cv::Mat& image2);

// Count the number of black pixels in an 8-bit image and also return the ratio of black to non-black pixels
int countBlackPixels(const cv::Mat& img, double& blackRatio);

// Set rgbImage value to 0 where its mask value is zero
void andWithMask_8UC3(cv::Mat& rgbImage, const cv::Mat& mask);
// Set srcMask (8-bit) value to 0 where its mask value is zero
void andWithMask_8U(cv::Mat& srcMask, const cv::Mat& mask);

void mulByMaskC3(const cv::Mat& srcImage, const cv::Mat& mask, cv::Mat& targetImage);
void mulByMaskC1(const cv::Mat& srcImage, const cv::Mat& mask, cv::Mat& targetImage);

void inpaintMaskedPixels(cv::Mat& image, const cv::Mat& mask, int filterSize);

enum ExtremaType {
	FIRST_MIN,
	FIRST_MAX,
	LAST_MAX,
	GLOBAL_MIN,
	GLOBAL_MAX
};

int findExtrema(const cv::Mat& linearMat, float threshold, ExtremaType extremaType, float valueScale, bool firstDer = true);

// Convert an array of hex encoded image data to a Mat
cv::Mat hexToImage(uchar* hexData, unsigned long dataSize);


// Find histogram min/max locations given min/max value thresholds
// histogram type must be CV_32FC1
void findHistogramMinMax(const cv::Mat& hist, int minThresh, int maxThresh, int& minLoc, int& maxLoc);

// Return the average color of srcImage
cv::Vec3b getMeanColor(const cv::Mat& srcImage, bool excludeBlack=false);

// Return the mean gray value of srcGray. srcGray must be 8-bit image
double getMeanGray(const cv::Mat& srcGray);

// change the brightness of srcImage to match targetImage
// if matchRect is set, only match the region inside the rect
void matchImageBrightness(cv::Mat& srcImage, const cv::Mat& targetImage, const cv::Rect& matchRect=cv::Rect());

// change srcImage average color to match the targetImage average color 
// if mask is not empty, only match the non zero mask pixels
void matchTargetColor(cv::Mat& srcImage, const cv::Mat& targetImage, const cv::Mat& mask = cv::Mat());

// Return true if the file is ascii STL file
bool isAsciiSTL(const cv::String& stlFile);

// Return a bounding rect of non-zero pixels in a 8-bit mask image
cv::Rect getMaskBoundRect(const cv::Mat& maskImage);

// sharpening an image with unsharp masking
// https://stackoverflow.com/questions/4993082/how-to-sharpen-an-image-in-opencv
void sharpenImage(const cv::Mat& srcImage, cv::Mat& targetImage, double amount = 0.5);


// get binlinearly interpolated values of point x, y in image
// fx, fy are fractional offset from x, y
// stride is the image row size (number of items in a row)
// T can be Point, Point2f, Point3f, Vec2f, Vec3f
template<class T>
static T bilinearInterp(const cv::Mat& image, int x, int y, float fx, float fy, int stride) {

	const int c1 = image.cols - 1;
	const int r1 = image.rows - 1;

	if (x<0 || x>c1 || y<0 || y>r1) {
		return T();
	}

	const T* p = image.ptr<T>(y, x); // pointer to first pixel

	T v[4];
	v[0] = p[0];  // top-left
	v[1] = x < c1 ? p[1] : v[0];
	v[2] = y< r1 ? p[stride] : v[0];  // bot-left
	v[3] = x<c1 && y<r1 ? p[1 + stride] : v[0];  // bot-right

	float w[4];

	// Calculate the weights for each pixel
	const float fx1 = 1.0f - fx;
	const float fy1 = 1.0f - fy;

	w[0] = fx1 * fy1;
	w[1] = fx  * fy1;
	w[2] = fx1 * fy;
	w[3] = fx  * fy;


	return (v[0]*w[0]+v[1]*w[1]+v[2]*w[2]+v[3]*w[3]);
}

// replace chroma key colored pixels in image with replacementColor
// RGB images only
void replaceChromaKeyColor(cv::Mat& image, cv::Vec3b replacementColor);

void convertDepthToContourMapC3(const cv::Mat& depthMap, cv::Mat& outMap, double stepSize= 200);

// return a mask from a rgb image where pixels with chroma key color have non zero mask values
cv::Mat getChromaKeyMask(cv::Mat& image);

// set pixels in image outside of r to color
void setOutside(cv::Mat& image, cv::Rect r, cv::Scalar color=0);

// find the longest contour is a vector of contours
int findLongestContour(const std::vector<std::vector<cv::Point>>& contours);

// compute per pixel filter size from X Y gradients
void computeFilterMap(const cv::Mat& xyMap, cv::Mat& filterMap);

// fill dstImg pixels with srcImg pixels using dst2SrcMap
// dst2SrcMap should be 32FC2 
// srcImage must be 8UC3
// dstImg will have the same size as dst2SrcMap
void remap8UC3(const cv::Mat& srcImg, cv::Mat& dstImg, const cv::Mat& dst2SrcMap, 
	const cv::Mat& filterMap=cv::Mat());

// srcImg must be 8UC1
void remap8U(const cv::Mat& srcImg, cv::Mat& dstImg, const cv::Mat& dst2SrcMap);

} // namespace b3di