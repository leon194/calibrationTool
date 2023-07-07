#include "B3D4AutoCalib.h"

#include <stdio.h>
#include <unordered_set>

#include "b3di/TLog.h"
#include "b3di/B3d_utils.h"

#include "b3ddepth/core/ext/DepthConfigExt.h"
#include "b3ddepth/utils/ext/B3DCalibrationData.h"

#include "XForm.h"

#include <opencv2/calib3d.hpp>

#ifdef USE_OMP
#include <omp.h>
#endif

// Not used. Require extra Eigen module from C++ 11.
#ifdef FIT_PLANE
#include <Eigen/Core>
#include "SimpleRansac.h"
#include "PlaneModel.h"
#endif

#ifdef __ANDROID__
#include "backTrace.h"
#endif

using namespace std;
using namespace cv;
using namespace b3di;
using namespace trimesh;


// OpenCV stereoBM uses fixed point disparity with 4 fractional bits
const int DISPARITY_SHIFT = 4;
// Need to scale the coordinates by 16 because stereorectify creates disparity with 4 bits of fractional points
const double Z_SCALE = 1 << DISPARITY_SHIFT;

//const int MAX_NUM_FEATURES = 400;
//const int MIN_NUM_FEATURES = 150;
namespace b3d4 {

B3D4AutoCalib::B3D4AutoCalib() {
#ifdef __ANDROID__
    signal(SIGSEGV, signalHandler);
#endif
}

B3D4AutoCalib::~B3D4AutoCalib() {

}

static
bool calibrateCameraWithReference(const vector<vector<cv::Point3f>> &refPoints,
                                  const vector<vector<cv::Point2f>> &imagePoints,
                                  const CameraParams &imageCam, CameraParams &computedCam,
                                  vector<xform> &camXforms, double &calibErr,
                                  const int calibFlags) {
    const int MIN_CALIB_MATCHES = 15;
    const TermCriteria k_criteria = TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30,
                                                 DBL_EPSILON);

    Mat intrinsic = imageCam.getIntrinsicMat();
    //intrinsic.at<double>(0, 2) = cam_i.getImageSize().width*0.5;
    //intrinsic.at<double>(1, 2) = cam_i.getImageSize().height*0.5;
    Mat distCoeffs = imageCam.getDistCoeffs();
    vector<Mat> rvecs;
    vector<Mat> tvecs;
    //			int flags = 0;
    //		int flags = CV_CALIB_USE_INTRINSIC_GUESS | CV_CALIB_ZERO_TANGENT_DIST | CV_CALIB_FIX_PRINCIPAL_POINT |
    //			CV_CALIB_FIX_K1 | CV_CALIB_FIX_K2 | CV_CALIB_FIX_K3 | CV_CALIB_FIX_FOCAL_LENGTH;
    //		int flags = CV_CALIB_USE_INTRINSIC_GUESS | CV_CALIB_ZERO_TANGENT_DIST |
    //			CV_CALIB_FIX_K1 | CV_CALIB_FIX_K2 | CV_CALIB_FIX_K3 | CV_CALIB_FIX_FOCAL_LENGTH;
    //		int flags = CV_CALIB_USE_INTRINSIC_GUESS | CV_CALIB_ZERO_TANGENT_DIST;
    int flags = calibFlags;

    calibErr = cv::calibrateCamera(refPoints, imagePoints, imageCam.getImageSize(), intrinsic,
                                   distCoeffs, rvecs, tvecs, flags, k_criteria);

    // Set the intrinsic params for the camera
    computedCam.setDistCoeffs(distCoeffs);
    computedCam.setIntrinsic(intrinsic.at<double>(0, 0), intrinsic.at<double>(1, 1),
                             intrinsic.at<double>(0, 2), intrinsic.at<double>(1, 2),
                             computedCam.getImageSize());
    //		cam_i.setImageSize(inputCams[i].getImageSize());

    // Get a list of transformations for each set of points
    camXforms.resize(refPoints.size());

    Vec3d rot, tra;
    int max_j_num = 0;
    int max_j_index = -1;
    for (int j = 0; j < refPoints.size(); j++) {

        // Find the image that has the most matching pairs of points
        if (refPoints[j].size() > max_j_num) {
            max_j_num = (int) refPoints[j].size();
            max_j_index = j;
        }

        // Get the xform matrix from rotation and translation vectors
        Mat R_matrix;
        Rodrigues(rvecs[j], R_matrix);
        double *rmtx = R_matrix.ptr<double>(0);

        camXforms[j] = xform(
                rmtx[0], rmtx[3], rmtx[6], 0.0,
                rmtx[1], rmtx[4], rmtx[7], 0.0,
                rmtx[2], rmtx[5], rmtx[8], 0.0,
                tvecs[j].at<double>(0), tvecs[j].at<double>(1), tvecs[j].at<double>(2), 1.0
        );

    }

    max_j_index = 0;
    if (max_j_index < 0 || refPoints[max_j_index].size() < MIN_CALIB_MATCHES) {
        TLog::log(TLog::LOG_ERROR, "Insufficient matching points found");
        camXforms.clear();
        return false;
    }


    return true;

}

static
void computeExtrinsicRelativeToTarget(const vector<xform> &inputCamXform,
                                      const vector<xform> &targetCamXform,
                                      xform &computedXform) {
    CV_Assert(inputCamXform.size() == targetCamXform.size());
    CV_Assert(inputCamXform.size() > 0);
    int n = (int) inputCamXform.size();

    // Get the average R and T vectors relative to the targetCam from the inputCamXform
    Vec3d rot, tra;
    int max_j_num = 0;
    int max_j_index = -1;
    for (int j = 0; j < n; j++) {

        // Concatenate xforms to get the relative transformation to targetCam
        xform xf_pj = inputCamXform[j];
        xform xf_0p = inv(targetCamXform[j]);
        xform xf = xf_pj * xf_0p;
        Vec3d rot_j, tra_j;
        xformToDof6(xf, rot_j, tra_j);
        rot += rot_j;
        tra += tra_j;
    }

    rot /= (double) n;
    tra /= (double) n;

    computedXform = Dof6ToXform(rot, tra);

}

static void drawRectifiedImages(const Mat &imgL_rect, const Mat &imgR_rect) {

    Rect lRect(0, 0, imgL_rect.cols, imgL_rect.rows);
    Rect rRect(imgL_rect.cols, 0, imgL_rect.cols, imgL_rect.rows);
    Mat imgLR(imgL_rect.rows, imgL_rect.cols * 2, CV_8UC1);

    imgL_rect.copyTo(imgLR(lRect));
    imgR_rect.copyTo(imgLR(rRect));

    drawHLines(imgLR, 50, Scalar::all(255));
    showImage("Rectified", imgLR, 1280);

    //Mat img1d, img2d;
    //if (img1r.rows > MAX_WIN_H) {
    //	cv::resize(img1r, img1d, Size(), 0.5, 0.5);
    //	cv::resize(img2r, img2d, Size(), 0.5, 0.5);
    //}
    //else {
    //	img1d = img1r.clone();
    //	img2d = img2r.clone();
    //}

    //drawHLines(img1d, 50, Scalar::all(255));
    //drawHLines(img2d, 50, Scalar::all(255));

    //cv::imshow("Rectified0", img1d);
    //cv::imshow("Rectified1", img2d);
    //imwrite(dataDir+"rectified0.png", img1r, compression_params);
    //imwrite(dataDir + "rectifiedLR.png", imgLR, compression_params);

}

static void drawDisparityMap(const Mat &dispMap) {
    double minVal;
    double maxVal;
    minMaxLoc(dispMap, &minVal, &maxVal);

    Mat disp8;
    //convert16ToRainbowColor(disp, disp8);
    //		disp.convertTo(disp8, CV_8UC1, 0.04);
    dispMap.convertTo(disp8, CV_8UC1, 255.0 / maxVal);
    //if (disp8.rows > MAX_WIN_H)
    //	cv::resize(disp8, disp8, Size(), 0.5, 0.5);
    showImage("disparityMap", disp8);
}


//// Compute disparity map from L/R images and also return
//// rectified images: imgL_rect, imgR_rect, offset maps: mapL, mapR, and rectification params R1, R2, Q
//static
//void getDisparityMap(const Mat& imageL, const Mat& imageR,
//	const CameraParams& camL, const CameraParams& camR,
//	const Mat& R, const Mat& T, int maxDisparity, int blockSize,
//	Mat& dispMap, Mat& imgL_rect, Mat& imgR_rect, 
//	Mat& mapL, Mat& mapR, Mat& R1, Mat& R2, Mat& Q)
//{
//
//	Size img_size = imageL.size();
//
//	Rect roi1, roi2;
//
//	Mat M1, D1, M2, D2;
//	M1 = camL.getIntrinsicMat();
//	D1 = camL.getDistCoeffs();
//	M2 = camR.getIntrinsicMat();
//	D2 = camR.getDistCoeffs();
//
//	Mat P1, P2;
//	double alpha = -1.0;
//
//	cv::stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, alpha, img_size, &roi1, &roi2);
//
//	// Compute rotation vector of the rectified left image
//	Mat rvec1;
//	cv::Rodrigues(R1, rvec1);
//	Mat rvec2;
//	cv::Rodrigues(R2, rvec2);
//
//	Mat map1r[2], map2r[2];
//	//cv::initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_32FC2, map1r[0], map1r[1]);
//	//cv::initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_32FC2, map2r[0], map2r[1]);
//
//	cv::initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map1r[0], map1r[1]);
//	cv::initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map2r[0], map2r[1]);
//
//	mapL = map1r[0];
//	mapR = map2r[0];
//
//	Mat img1r, img2r;
//	cv::remap(imageL, img1r, map1r[0], map1r[1], INTER_LINEAR);
//	cv::remap(imageR, img2r, map2r[0], map2r[1], INTER_LINEAR);
//	
//	int numberOfDisparities = maxDisparity;
//	numberOfDisparities = ((int)ceil(numberOfDisparities / 16.0)) * 16;
//
//	// StereoBM seems to not compute the disparity on the left side of the image
//	// and the width of the empty region equals to numberOfDisparities
//	// Resize the image to add numberOfDisparities columns padding to the left of the image
//	Mat temp1 = Mat::zeros(img1r.rows, img1r.cols + numberOfDisparities, img1r.type());
//	Rect tRect1(numberOfDisparities, 0, img1r.cols, img1r.rows);
//	img1r.copyTo(temp1(tRect1));
//	img1r = temp1;
//	Mat temp2 = Mat::zeros(img2r.rows, img2r.cols + numberOfDisparities, img2r.type());
//	Rect tRect2(numberOfDisparities, 0, img2r.cols, img2r.rows);
//	img2r.copyTo(temp2(tRect2));
//	img2r = temp2;
//
//
//
//	Ptr<StereoBM> bm;
//	int bmBlockSize;
//	int cn = img1r.channels();
//
//	bm = StereoBM::create();
//
//	bmBlockSize = blockSize;
//	bm->setROI1(roi1);
//	bm->setROI2(roi2);
//	bm->setBlockSize(bmBlockSize);
//	bm->setMinDisparity(1);		// must be 1 so <=0 indicates invalid disparity value
//	bm->setNumDisparities(numberOfDisparities);
//	bm->setUniquenessRatio(5);
//	bm->setSpeckleWindowSize(100);
//	bm->setSpeckleRange(32);
//	bm->setDisp12MaxDiff(1);
//
//	bm->compute(img1r, img2r, dispMap);
//
//
//	// Remove the extra padding space on the left side
//	Rect tRect(numberOfDisparities, 0, img1r.cols - numberOfDisparities, img1r.rows);
//	dispMap = dispMap(tRect);
//	imgL_rect = img1r(tRect);
//	imgR_rect = img2r(tRect);
//
////#ifdef _DEBUG
////
////	double minVal; double maxVal;
////	minMaxLoc(dispMap, &minVal, &maxVal);
////
////	Mat disp8;
////	//convert16ToRainbowColor(disp, disp8);
////	//		disp.convertTo(disp8, CV_8UC1, 0.04);
////	dispMap.convertTo(disp8, CV_8UC1, 255.0 / maxVal);
////	//if (disp8.rows > MAX_WIN_H)
////	//	cv::resize(disp8, disp8, Size(), 0.5, 0.5);
////	cv::imshow("disparityMap", disp8);
////	cv::waitKey(100);
////
////	Rect lRect(0, 0, imgL_rect.cols, imgL_rect.rows);
////	Rect rRect(imgL_rect.cols, 0, imgL_rect.cols, imgL_rect.rows);
////	Mat imgLR(imgL_rect.rows, imgL_rect.cols * 2, CV_8UC1);
////
////	imgL_rect.copyTo(imgLR(lRect));
////	imgR_rect.copyTo(imgLR(rRect));
////
////	drawHLines(imgLR, 50, Scalar::all(255));
////	showImage("RectifiedLR", imgLR, 1280);
////
////	//Mat img1d, img2d;
////	//if (img1r.rows > MAX_WIN_H) {
////	//	cv::resize(img1r, img1d, Size(), 0.5, 0.5);
////	//	cv::resize(img2r, img2d, Size(), 0.5, 0.5);
////	//}
////	//else {
////	//	img1d = img1r.clone();
////	//	img2d = img2r.clone();
////	//}
////
////	//drawHLines(img1d, 50, Scalar::all(255));
////	//drawHLines(img2d, 50, Scalar::all(255));
////
////	//cv::imshow("Rectified0", img1d);
////	//cv::imshow("Rectified1", img2d);
////	//imwrite(dataDir+"rectified0.png", img1r, compression_params);
////	//imwrite(dataDir + "rectifiedLR.png", imgLR, compression_params);
////
////	cv::waitKey(100);
////#endif
//
//}


// Get feature points form a rectified L image and 
// find their corresponding points in R image using a disparity map
// Return featuresL and featuresR, feature point coordinates for L and R images
// Return the number of valid feature points (non-zero values)
static
int geMatchingFeaturesFromDisparity(const Mat &imgL_rect, const Mat &dispMap,
                                    vector<Point2f> &featuresL, vector<Point2f> &featuresR) {
    // Find feature points that have valid disparity value in L image
    featuresL.clear();
    featuresR.clear();

    // Determine number features to track use the image size
    //	int maxFeatures = (imgL_rect.rows*imgL_rect.cols) / 1200;
    int maxFeatures = (imgL_rect.rows * imgL_rect.cols) / 1000;
    if (maxFeatures < 150)
        maxFeatures = 150;

    int minFeatures = maxFeatures / 2;
    if (minFeatures > 150)
        minFeatures = 150;

    // Use GOOD features to track
    cv::goodFeaturesToTrack(imgL_rect, featuresL, maxFeatures, 0.01, 15);
    //TermCriteria termcrit(TermCriteria::COUNT | TermCriteria::EPS, 20, 0.03);
    //Size subPixWinSize(8, 8);
    //cornerSubPix(camImages[0], featurePoints[0], subPixWinSize, Size(-1, -1), termcrit);
    //std::cout << " Number of good features= " << (int)featuresL.size() << std::endl;

    if (featuresL.size() < minFeatures) {
        TLog::log(TLog::LOG_ERROR,
                  "Insufficient features points found. Requested=%d, minNeeded=%d, found=%d",
                  maxFeatures, minFeatures, (int) featuresL.size());

        featuresL.resize(0);
        return 0;
    }

    vector<Point3f> dispPoints(featuresL.size());

    // Construct disparity points with the disparity value as z
    for (int i = 0; i < (int) featuresL.size(); i++) {
        const Point2f &srcPt = featuresL[i];
        double zval = dispMap.at<short>((int) srcPt.y, (int) srcPt.x) / Z_SCALE;
        dispPoints[i] = Point3f(srcPt.x, srcPt.y, (float) zval);
    }

#ifdef FIT_PLANE

    std::vector<Eigen::Vector3d> planPoints(dispPoints.size());
    // Copy points
    for (int i = 0; i < (int)dispPoints.size(); i++)
    {
        const Point3f& srcPt = dispPoints[i];
        planPoints[i] = Eigen::Vector3d(srcPt.x, srcPt.y, srcPt.z);
    }

    // Fit a plane to the disparity points
    int numIter = 300;
    int plInliers = planPoints.size()*0.7;
    double thresh = 0.5;

    const PlaneModel m = ransac<PlaneModel>(planPoints, thresh, numIter, plInliers);
    if (m.d > 0) {
        Eigen::Vector3d n = m.n.transpose();
        std::cout << "Computed plane: n=" << n << " d=" << m.d
            << " inliers=" << plInliers << " iters=" << numIter << std::endl;

        const double nx = (double)n[0];
        const double ny = (double)n[1];
        const double nz = (double)n[2];

        const double pA = -(nx / nz);
        const double pB = -(ny / nz);
        const double pC = -(m.d / nz);

        // Refine the disparity value using the computed plane equation
        featurePoints[1].resize(planPoints.size());
        for (int i = 0; i < (int)planPoints.size(); i++)
        {
            const Eigen::Vector3d& pt = planPoints[i];
            // compute disparity from the plane equation
            dispPoints[i].z = (float)(pA*pt[0] + pB*pt[1] + pC);
        }
    }
    else {
        std::cerr << "(!) Plane fitting failed." << std::endl;
    }
#endif

    // Compute the matching feature points in R image using the disparity value
    // Assume valid points have positive disparity values
    featuresR.resize(dispPoints.size());
    int validPointCount = 0;
    for (int i = 0; i < (int) dispPoints.size(); i++) {
        const Point3f &pt = dispPoints[i];
        if (pt.z > 0) {
            Point2f pt2;
            pt2.x = pt.x - pt.z;
            pt2.y = pt.y;
            featuresR[i] = pt2;
            validPointCount++;
        } else {
            // Invalid disparity value
            // Indicate no matching point
            featuresR[i] = Point2f(-1, -1);
        }
    }

    return validPointCount;
}

// Compute 3d disparity points (x/y are pixel coordinates, z valus is disparity value) 
// from featuresL and find their matching image points in R image=
// featuresR provided the initial estimate of the R image points
// Use template matching to search a small neighborhood (searchSize) to find a point that match featuresL
static
double getDisparityPointsFromFeatures(const vector<Point2f> &featuresL,
                                      const vector<Point2f> &featuresR,
                                      const Mat &imgL, const Mat &imgR, const Mat &mapL,
                                      const Mat &mapR,
                                      int templSize, int searchSize,
                                      vector<Point3f> &dispPoints3d, vector<Point2f> &imgLPts,
                                      vector<Point2f> &imgRPts,
                                      Vec2d &avgOffset) {
    CV_Assert(mapL.type() == CV_16SC2 || mapL.type() == CV_32FC2);
    CV_Assert(mapR.type() == mapL.type());

    dispPoints3d.clear();
    imgLPts.clear();
    imgRPts.clear();

    // Use template matching to refine the match in R image
    int imgWidth = imgL.cols;
    int imgHeight = imgL.rows;
    Mat resultImg;
    int matchMethod = CV_TM_CCORR_NORMED;
    const int tx1 = imgWidth - templSize - 1;
    const int ty1 = imgHeight - templSize - 1;
    const int sx1 = imgWidth - searchSize - 1;
    const int sy1 = imgHeight - searchSize - 1;
    int templWinSize = 2 * templSize + 1;
    int searchWinSize = 2 * searchSize + 1;

    float offset = (searchWinSize - templWinSize) / 2.0f;

#ifdef _SHOW_DBG
    Mat showImgL = imgL.clone();
    Mat showImgR = imgR.clone();
#endif

    double dispError = 0;
     double xDiff = 0, yDiff = 0;
     for (int i = 0; i < (int) featuresL.size(); i++) {
         const Point2f &pt1 = featuresL[i];
         Point2f pt2 = featuresR[i];
         if (pt1.x >= templSize && pt1.x < tx1 &&
             pt1.y >= templSize && pt1.y < ty1 &&
             pt2.x >= searchSize && pt2.x < sx1 &&
             pt2.y >= searchSize && pt2.y < sy1) {
             // Find a point in imgR that matches the same point in imgL using the disparity as the initial estimate
             Rect templRoi((int) (pt1.x - templSize + 0.5f), (int) (pt1.y - templSize + 0.5f),
                           templWinSize, templWinSize);
             Rect targetRoi((int) (pt2.x - searchSize + 0.5f), (int) (pt2.y - searchSize + 0.5f),
                            searchWinSize, searchWinSize);
             Mat templImg = imgL(templRoi);
             Mat targetImg = imgR(targetRoi);

             // Use cross correlation for matching
             matchTemplate(targetImg, templImg, resultImg, matchMethod);
             //normalize(resultImg, resultImg, 0, 1, NORM_MINMAX, -1, Mat());
             double minVal, maxVal;
             Point minLoc, maxLoc;
             minMaxLoc(resultImg, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
             Point mathLoc;
             /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
             if (matchMethod == CV_TM_SQDIFF || matchMethod == CV_TM_SQDIFF_NORMED) {
                 mathLoc = minLoc;
             } else {
                 mathLoc = maxLoc;
             }


             // compute the new point
             Point2f pt2n;
             pt2n.x = pt2.x + mathLoc.x - offset;
             pt2n.y = pt2.y + mathLoc.y - offset;
             //pt2n = pt2;
#ifdef _SHOW_DBG
             circle(showImgL, pt1, 5, 255, 1);
             circle(showImgR, pt2, 5, 128, 1);
             circle(showImgR, pt2n, 5, 255, 1);
#endif
             // Compute the error between the original and new point
             //double ptDist = norm(pt2n - pt2);
             //xDiff += fabs(pt2n.x - pt2.x);
             //yDiff += fabs(pt2n.y - pt2.y);
             xDiff += pt2n.x - pt2.x;
             yDiff += pt2n.y - pt2.y;
             //dispError += (float)ptDist;
             dispError += fabs(pt2n.y - pt2.y);

             // Save x/y/disparity so we can compute the 3d point later
             float dispVal = (float) ((pt1.x - pt2.x) * Z_SCALE);
             //			float dispVal = (float)(pt1.x - pt2n.x)*zScale;
             if (dispVal < 0) dispVal = 0;
             Point3f dispPt = Point3f(pt1.x, pt1.y, dispVal);

             // Find the image point in the unrectified L and R image space
             Point2f pt1u, pt2u;
             if (mapL.type() == CV_16SC2) {
                 short *p1 = (short *) mapL.ptr<long>((int) (pt1.y + 0.5f),
                                                      (int) (pt1.x + 0.5f));
                 pt1u = Point2f((float) p1[0], (float) p1[1]);
                 short *p2 = (short *) mapR.ptr<long>((int) (pt2n.y + 0.5f),
                                                      (int) (pt2n.x + 0.5f));
                 pt2u = Point2f((float) p2[0], (float) p2[1]);
             } else {
                 float *p1 = (float *) mapL.ptr<double>((int) (pt1.y + 0.5f),
                                                        (int) (pt1.x + 0.5f));
                 pt1u = Point2f(p1[0], p1[1]);
                 float *p2 = (float *) mapR.ptr<double>((int) (pt2n.y + 0.5f),
                                                        (int) (pt2n.x + 0.5f));
                 pt2u = Point2f(p2[0], p2[1]);
             }
             //// this is for type=CV_32FC2
             ////			float* p1 = (float*)mapL.ptr<double>((int)(pt1.y + 0.5f), (int)(pt1.x + 0.5f));
             //			short* p1 = (short*)mapL.ptr<long>((int)(pt1.y + 0.5f), (int)(pt1.x + 0.5f));
             //			Point2f pt1u((float)p1[0], (float)p1[1]);
             ////			float* p2 = (float*)mapR.ptr<double>((int)(pt2n.y + 0.5f), (int)(pt2n.x + 0.5f));
             //			short* p2 = (short*)mapR.ptr<long>((int)(pt2n.y + 0.5f), (int)(pt2n.x + 0.5f));
             //			Point2f pt2u((float)p2[0], (float)p2[1]);

             dispPoints3d.push_back(dispPt);
             imgLPts.push_back(pt1u);
             imgRPts.push_back(pt2u);
         }
     }

#ifdef _SHOW_DBG
     showImage("RectifiedL", showImgL);
     showImage("RectifiedR", showImgR);
     cv::waitKey(30);
#endif

     avgOffset[0] = xDiff;
     avgOffset[1] = yDiff;
     if (dispPoints3d.size() > 0) {
         dispError /= (double) dispPoints3d.size();
         avgOffset[0] /= (double) dispPoints3d.size();
         avgOffset[1] /= (double) dispPoints3d.size();
     }

     //std::cout << "Disparity error=" << dispError << " xDiff=" << avgOffset[0] << " yDiff=" << avgOffset[1] << endl;

     //showImgL = camImages[0].clone();
     //showImgR = camImages[1].clone();

     //for (int i = 0; i < (int)plPts2d[0].size(); i++)
     //{
     //	const Point2f& pt1 = plPts2d[0][i];
     //	const Point2f& pt2 = plPts2d[1][i];

     //	circle(showImgL, pt1, 5, _WHITE, 1);
     //	circle(showImgR, pt2, 5, _WHITE, 1);
     //}

     //cv::imshow("ImageL", showImgL);
     //cv::imshow("ImageR", showImgR);
     //cv::waitKey(100);

     return dispError;
}


static
bool computeExtrinsicFormDisparityPoints(const vector<Point3f> &dispPoints,
                                         const vector<Point2f> imgPts[2], const Mat &R1,
                                         const Mat &Q,
                                         CameraParams camParams[2]) {
    // Convert disparity to 3d points
    static vector<Point3f> objPts;
    objPts.clear();
    cv::perspectiveTransform(dispPoints, objPts, Q);

    xform xfR1 = mat3x3ToXform(R1);
    xfR1 = inv(xfR1);

    xform transform = xfR1 * xform::scale(Z_SCALE);

    xformPoints3df(objPts, transform);

    //Point3d plCentr(0, 0, 0);
    //for (int i = 0; i < (int)plPts3d.size(); i++)
    //{
    //	plCentr += (Point3d)plPts3d[i];

    //}
    //plCentr /= (double)plPts3d.size();
    //std::cout << "Plane centroid=" << plCentr << endl;

    // Use 3d-to-2d matching to solve for camera calibration

    // To hold computed camera transformations relative to the calibration image
    // Each entry contains a vector of transformations correspoing to the number of image files per camera
    vector<vector<trimesh::xform>> camXforms(2);

    int calibFlags =
            CV_CALIB_USE_INTRINSIC_GUESS
            | CV_CALIB_FIX_TANGENT_DIST
            | CV_CALIB_FIX_FOCAL_LENGTH
            | CV_CALIB_FIX_K1 | CV_CALIB_FIX_K2 | CV_CALIB_FIX_K3
            | CV_CALIB_FIX_PRINCIPAL_POINT;

    // Compute intrinsic params for L and R cam
    for (int i = 0; i < 2; i++) {

        vector<vector<cv::Point3f>> object_points; // container for the model 3D coordinates of the good matches
        vector<vector<cv::Point2f>> image_points; // container for the image 2D coordinates of the good matches
        object_points.push_back(objPts);
        image_points.push_back(imgPts[i]);

        double err;
        vector<trimesh::xform> imageXforms;
        CameraParams compCam = camParams[i];

        bool ret = calibrateCameraWithReference(object_points, image_points, camParams[i],
                                                compCam, imageXforms, err, calibFlags);

        if (ret) {
            camParams[i] = compCam;
            camXforms[i] = imageXforms;

            //PnPSolver pnpSolver(camParams[i].getIntrinsicMat(), &camParams[i].distCoeffs,
            //	camParams[i].getRMatrix(), camParams[i].getTMatrix());

            ////pnpSolver.estimatePose(objPts, imgPts[i], SOLVEPNP_ITERATIVE);

            //Mat inliers_idx;
            //pnpSolver.estimatePoseRANSAC(objPts, imgPts[i],
            //	SOLVEPNP_ITERATIVE, inliers_idx,
            //	2500, 1.5, 0.95);

            //cout << "solvePnP input points="<< objPts.size()<<" inliers=" << inliers_idx.rows << endl;

            //camXforms[i].resize(1);
            //camXforms[i][0] = pnpSolver.getEstimatedXform();

            //std::cout << "Computed intrinsic cam=" << i << ", " <<
            //	compCam.fx << ", " << compCam.fy << ", " << compCam.cx << ", " << compCam.cy
            //	<< ", " << compCam.distCoeffs.at<double>(0, 0)
            //	<< ", " << compCam.distCoeffs.at<double>(0, 1)
            //	<< ", " << compCam.distCoeffs.at<double>(0, 2)
            //	<< ", " << compCam.distCoeffs.at<double>(0, 3)
            //	<< ", " << compCam.distCoeffs.at<double>(0, 4)
            //	<< endl;
        } else {
            return false;
        }
    }

    // Compute extrinsic of R cam relative to the L cam
    xform computedXf;
    computeExtrinsicRelativeToTarget(camXforms[1], camXforms[0], computedXf);

    camParams[0].setWorldToCameraXform(xform::identity());
    camParams[1].setWorldToCameraXform(computedXf);
    //Vec3d rot, tra;
    //xformToDof6(computedXf, rot, tra);

    return true;
}

static
bool computeExtrinsicFormRandDisparityPoints(const vector<Point3f> &rawDispPoints,
    const vector<Point2f> rawImgPts[2], const Mat &R1,
    const Mat &Q,
    CameraParams camParams[2],
    float targetPtRatio
) {
    // rand select pts from input data
    const int rawPtNum = static_cast<int>(rawDispPoints.size());
    targetPtRatio = targetPtRatio > 0.9f ? 0.9f :
        targetPtRatio < 0.7f ? 0.7f : targetPtRatio;

    const int minPtNum = 225;
    const int targetPtNum = static_cast<int>(std::round(rawPtNum * targetPtRatio));
    if (targetPtNum < minPtNum)
    {
        TLog::log(TLog::LOG_WARNING,
            "computeExtrinsicFormRandDisparityPoints Insuffcient number of subset feature points subset[%d] min[%d]",
            targetPtNum, minPtNum);
        return false;
    }

    std::unordered_set<int> selectIndices;
    selectIndices.reserve(targetPtNum);
    vector<Point3f> dispPoints;
    dispPoints.reserve(targetPtNum);
    vector<Point2f> imgPts[2];
    imgPts[0].reserve(targetPtNum);
    imgPts[1].reserve(targetPtNum);
    while (selectIndices.size() != targetPtNum)
    {
        const int idx = rand() % rawPtNum;
        if (selectIndices.find(idx) == selectIndices.end())
        {
            selectIndices.insert(idx);
            dispPoints.push_back(rawDispPoints[idx]);
            imgPts[0].push_back(rawImgPts[0][idx]);
            imgPts[1].push_back(rawImgPts[1][idx]);
        }
    }

    // Convert disparity to 3d points
    static vector<Point3f> objPts;
    objPts.clear();
    cv::perspectiveTransform(dispPoints, objPts, Q);

    xform xfR1 = mat3x3ToXform(R1);
    xfR1 = inv(xfR1);

    xform transform = xfR1 * xform::scale(Z_SCALE);

    xformPoints3df(objPts, transform);

    std::vector<cv::Point3f> tmpObjPts;
    std::vector<cv::Point2f> tmpPts_0, tmpPts_1;
    for (int i = 2; i < objPts.size(); i += 3)
    {
        tmpObjPts.push_back(objPts[i]);
        tmpPts_0.push_back(imgPts[0][i]);
        tmpPts_1.push_back(imgPts[1][i]);
    }
    objPts = tmpObjPts;
    imgPts[0] = tmpPts_0;
    imgPts[1] = tmpPts_1;

    //Point3d plCentr(0, 0, 0);
    //for (int i = 0; i < (int)plPts3d.size(); i++)
    //{
    //	plCentr += (Point3d)plPts3d[i];

    //}
    //plCentr /= (double)plPts3d.size();
    //std::cout << "Plane centroid=" << plCentr << endl;

    // Use 3d-to-2d matching to solve for camera calibration

    // To hold computed camera transformations relative to the calibration image
    // Each entry contains a vector of transformations correspoing to the number of image files per camera
    vector<vector<trimesh::xform>> camXforms(2);

    int calibFlags =
        CV_CALIB_USE_INTRINSIC_GUESS
        | CV_CALIB_FIX_TANGENT_DIST
        | CV_CALIB_FIX_FOCAL_LENGTH
        | CV_CALIB_FIX_K1 | CV_CALIB_FIX_K2 | CV_CALIB_FIX_K3
        | CV_CALIB_FIX_PRINCIPAL_POINT;

    // Compute intrinsic params for L and R cam
    for (int i = 0; i < 2; i++) {

        vector<vector<cv::Point3f>> object_points; // container for the model 3D coordinates of the good matches
        vector<vector<cv::Point2f>> image_points; // container for the image 2D coordinates of the good matches
        object_points.push_back(objPts);
        image_points.push_back(imgPts[i]);

        double err;
        vector<trimesh::xform> imageXforms;
        CameraParams compCam = camParams[i];

        bool ret = calibrateCameraWithReference(object_points, image_points, camParams[i],
            compCam, imageXforms, err, calibFlags);

        if (ret) {
            auto a = camParams[i];
            camParams[i] = compCam;
            camXforms[i] = imageXforms;

            //PnPSolver pnpSolver(camParams[i].getIntrinsicMat(), &camParams[i].distCoeffs,
            //	camParams[i].getRMatrix(), camParams[i].getTMatrix());

            ////pnpSolver.estimatePose(objPts, imgPts[i], SOLVEPNP_ITERATIVE);

            //Mat inliers_idx;
            //pnpSolver.estimatePoseRANSAC(objPts, imgPts[i],
            //	SOLVEPNP_ITERATIVE, inliers_idx,
            //	2500, 1.5, 0.95);

            //cout << "solvePnP input points="<< objPts.size()<<" inliers=" << inliers_idx.rows << endl;

            //camXforms[i].resize(1);
            //camXforms[i][0] = pnpSolver.getEstimatedXform();

            //std::cout << "Computed intrinsic cam=" << i << ", " <<
            //	compCam.fx << ", " << compCam.fy << ", " << compCam.cx << ", " << compCam.cy
            //	<< ", " << compCam.distCoeffs.at<double>(0, 0)
            //	<< ", " << compCam.distCoeffs.at<double>(0, 1)
            //	<< ", " << compCam.distCoeffs.at<double>(0, 2)
            //	<< ", " << compCam.distCoeffs.at<double>(0, 3)
            //	<< ", " << compCam.distCoeffs.at<double>(0, 4)
            //	<< endl;
        }
        else {
            return false;
        }
    }

    // Compute extrinsic of R cam relative to the L cam
    xform computedXf;
    computeExtrinsicRelativeToTarget(camXforms[1], camXforms[0], computedXf);

    camParams[0].setWorldToCameraXform(xform::identity());
    camParams[1].setWorldToCameraXform(computedXf);
    //Vec3d rot, tra;
    //xformToDof6(computedXf, rot, tra);

    return true;
}

static void printProcessingInfo(b3dd::DepthConfigExtPtr depthConfigPtr) {

    b3dd::OPTIMIZE_MODE optMode = depthConfigPtr->optimizeMode;
    b3dd::DEPTH_REGISTRATION registerCam = depthConfigPtr->depthRegistration;
    double depthScale = depthConfigPtr->depthScale;
    double depthUnit = depthConfigPtr->depthUnit;


    TLog::log(TLog::LOG_ALWAYS, "App: ProcessingInfo: Optimize Mode: %s",
              (optMode == 0) ? "QUALITY" : "SPEED or STD");
    TLog::log(TLog::LOG_ALWAYS, "App: ProcessingInfo: Depth Space  :    %d (0:L, 1:R, 2:RGB)",
              registerCam);
    TLog::log(TLog::LOG_ALWAYS, "App: ProcessingInfo: Depth Scale  : %.2f (0.00 ~ 1.00)",
              depthScale);
    TLog::log(TLog::LOG_ALWAYS, "App: ProcessingInfo: Depth Unit   : %.2f (0.02 / 0.05)\n\n",
              depthUnit);

    string depthCamStr = getDepthCamTypeString(depthConfigPtr->depthCameraType);
    int minZ = depthConfigPtr->minScanDistance;
    int maxZ = depthConfigPtr->maxScanDistance;

    TLog::log(TLog::LOG_ALWAYS, "ProcessingInfo: DepthCameraType:       %s",
              depthCamStr.c_str());
    TLog::log(TLog::LOG_ALWAYS, "ProcessingInfo: min/max scan distance: %d~%d mm\n\n", minZ,
              maxZ);


    //TLog::log(TLog::LOG_INFO, "ProcessingInfo: USE SIMD: %d", USE_SIMD);
}

static void initDepthProcessor(
        b3dd::CameraCalibExtPtr cameraCalibExtPtr,
        b3dd::DepthProcessorExt depthProcessor,
        b3dd::ROI roi,
        b3dd::OPTIMIZE_MODE optimizeMpde = b3dd::OPTIMIZE_MODE::OPTIMIZE_FULL_RES,
        float depthScale = 1.0f
) {
    depthProcessor.loadCalibrationData(cameraCalibExtPtr);

    // depthConfigPtr settings
    b3dd::DepthConfigExtPtr depthConfigPtr =
            shared_ptr<b3dd::DepthConfigExt>(new b3dd::DepthConfigExt());
    depthConfigPtr->sceneMode = b3dd::SCENE_MODE::NORMAL;
    depthConfigPtr->depthRegistration = b3dd::DEPTH_REGISTRATION::REGISTER_L;

    depthConfigPtr->depthUnit = 0.02f;
    depthConfigPtr->useTwoDispMaps = false;
    depthConfigPtr->useDepthRange = false;
    depthConfigPtr->reduceDispBias = false;
    depthConfigPtr->useIRThres = true;
    depthConfigPtr->ROI_L = roi;

    depthConfigPtr->optimizeMode = optimizeMpde;
    depthConfigPtr->depthScale = depthScale;

    // update configs
    depthProcessor.updateProcessorSettingsExt(depthConfigPtr);

#ifdef _DEBUG
    printProcessingInfo(depthProcessor.getProcessorSettingsExt());
#endif // _DEBUG

    return;
}


// Convert opencv Mat to B3DImage(make a copy)
static void cvMatToB3DImage(const cv::Mat &cvMat, b3dd::B3DImage &b3dImage) {

    b3dd::B3DImageType type;

    switch (cvMat.type()) {
        case CV_8UC1:
            type = b3dd::B3DIMAGE_MONO;
            break;
        case CV_16UC1:
            type = b3dd::B3DIMAGE_DEPTH_16;
            break;
        case CV_8UC3:
            type = b3dd::B3DIMAGE_RGB;
            break;
        default:
            //logError("Unsupported B3DImageType: " + b3dd::to_string(type) + " !");
            return;
    }

    b3dImage = b3dd::B3DImage(cvMat.rows, cvMat.cols, type);
    std::memcpy(b3dImage.data(), cvMat.data,
                cvMat.rows * cvMat.cols * cvMat.elemSize() * sizeof(unsigned char));

    return;
}

// Convert B3DImage to opencv Mat (make a copy)
static // Convert B3DImage to opencv Mat (make a copy)
void B3DImageToCvMat(const b3dd::B3DImage &inImage, cv::Mat &outMat, b3dd::B3DImageType type) {

    int matType;

    switch (type) {
        case b3dd::B3DIMAGE_MONO:
            matType = CV_8UC1;
            break;
        case b3dd::B3DIMAGE_DEPTH_16:
            matType = CV_16UC1;
            break;
        case b3dd::B3DIMAGE_DISP_16:
            matType = CV_16SC1;
            break;
        case b3dd::B3DIMAGE_RGB:
            matType = CV_8UC3;
            break;
        default:
            //logError("Unsupported B3DImageType: " + b3dd::to_string(type) + " !");
            return;
    }

    outMat = Mat(inImage.rows(), inImage.cols(), matType);
    std::memcpy(outMat.data, inImage.data(),
                inImage.rowSize() * inImage.rows() * sizeof(unsigned char));
}

static void computeRectifyMaps(const b3di::CameraParams camParams[],
                               cv::Mat rectifyMaps[2][2], cv::Mat &R1, cv::Mat &Q) {
    Size img_size = camParams[0].getImageSize();

    Mat R, T;
    for (int i = 0; i < 2; ++i) {

        if (i == 1) {

            // R, T should be relative to the first camera
            // If the first camera is not at the origin, calculate the R and T
            //std::cout << "Cam0 T=" << camParams[0].getPosition()<< ", R="<< camParams[0].getRotation() << std::endl;
#ifdef ALLOW_LCAM_NON_IDENTITY
            if (camParams[0].getPosition() != Vec3d(0, 0, 0) || camParams[0].getRotation() != Vec3d(0, 0, 0)) {

                xform Mwi = camParams[i].getWorldToCameraXform();
                xform M0w = camParams[0].getCameraToWorldXform();
                xform M0i = Mwi*M0w;

                //                std::cout << "M0i=" << std::endl << M0i << std::endl;

                R = xformToMat3x3(M0i);

                vec3 pt0 = M0i*trimesh::vec3(0, 0, 0);
                T = Mat(3, 1, CV_64FC1);
                T.at<double>(0) = pt0[0];
                T.at<double>(1) = pt0[1];
                T.at<double>(2) = pt0[2];

                double rx, ry, rz;
                getRotationFromXform(M0i, rx, ry, rz);
                //std::printf("Camera: rotation=%.4f %.4f %.4f translation=%.4f %.4f %.4f \n",
                //    rx, ry, rz,
                //    pt0[0], pt0[1], pt0[2]
                //    );

            }
            else {
                R = camParams[1].getRMatrix();
                T = camParams[1].getTMatrix();
            }
#else
            // We assume the L cam is alway in identity space
            R = camParams[1].getRMatrix();
            T = camParams[1].getTMatrix();
#endif

        }
    }

    Mat M1, D1, M2, D2;
    M1 = camParams[0].getIntrinsicMat();
    D1 = camParams[0].getDistCoeffs();
    M2 = camParams[1].getIntrinsicMat();
    D2 = camParams[1].getDistCoeffs();

    //Mat R1, Q;
    Mat P1, R2, P2;
    const double rectify_alpha = -1.0;
    //	const double rectify_alpha = 0.0;
    cv::Rect roi1, roi2;

    cv::stereoRectify(M1, D1, M2, D2, img_size, R, T,
                      R1, R2,
                      P1, P2, Q,
                      CALIB_ZERO_DISPARITY, rectify_alpha, img_size,
                      &roi1, &roi2);

    // Seems to be more accurate to use floating point maps
    //	const int mType = CV_32FC2;
    const int mType = CV_16SC2;

    cv::initUndistortRectifyMap(M1, D1, R1, P1, img_size, mType,
                                rectifyMaps[0][0], rectifyMaps[0][1]);
    cv::initUndistortRectifyMap(M2, D2, R2, P2, img_size, mType,
                                rectifyMaps[1][0], rectifyMaps[1][1]);

}

// cvt cams to bin
    static bool cvtCameraParamsToB3DCalibBin(
            const std::vector<b3di::CameraParams> &cams,
            B3DCalibrationData &calibBin
    ) {
        if (cams.size() != 3) {
            return false;
        }

        /* for left cam */
        // left cam intrinsic
        calibBin.leftCamIntrinsic[0] = (float) cams[0].fx();
        calibBin.leftCamIntrinsic[1] = (float) cams[0].fy();
        calibBin.leftCamIntrinsic[2] = (float) cams[0].cx();
        calibBin.leftCamIntrinsic[3] = (float) cams[0].cy();

        // left cam dist coeffs
        for (int i = 0; i != 5; ++i) {
            calibBin.leftCamDistCoeff[i] = (float) cams[0].getDistCoeffs().at<double>(i, 0);
        }

        // left cam image size
        calibBin.leftCamImageSize[0] = (float) cams[0].getImageSize().width;
        calibBin.leftCamImageSize[1] = (float) cams[0].getImageSize().height;

        /* for right cam */
        // right cam instrinsic
        calibBin.rightCamIntrinsic[0] = (float) cams[1].fx();
        calibBin.rightCamIntrinsic[1] = (float) cams[1].fy();
        calibBin.rightCamIntrinsic[2] = (float) cams[1].cx();
        calibBin.rightCamIntrinsic[3] = (float) cams[1].cy();

        // right cam dist coeffs
        for (int i = 0; i != 5; ++i) {
            calibBin.rightCamDistCoeff[i] = (float) cams[1].getDistCoeffs().at<double>(i, 0);
        }

        // right cam rotation
        for (int i = 0; i != 9; ++i) {
            calibBin.rightCamRotation[i] = (float) cams[1].getRMatrix().at<double>(i / 3, i % 3);
        }

        // right cam translation
        for (int i = 0; i != 3; ++i) {
            calibBin.rightCamTranlation[i] = (float) cams[1].getTMatrix().at<double>(i, 0);
        }

        // right cam image size
        calibBin.rightCamImageSize[0] = (float) cams[1].getImageSize().width;
        calibBin.rightCamImageSize[1] = (float) cams[1].getImageSize().height;

        /* for mid cam */
        // mid cam intrisics
        calibBin.midCamIntrinsic[0] = (float) cams[2].fx();
        calibBin.midCamIntrinsic[1] = (float) cams[2].fy();
        calibBin.midCamIntrinsic[2] = (float) cams[2].cx();
        calibBin.midCamIntrinsic[3] = (float) cams[2].cy();

        // mid cam rotation
        for (int i = 0; i != 9; ++i) {
            calibBin.midCamRotation[i] = (float) cams[2].getRMatrix().at<double>(i / 3, i % 3);
        }

        // mid cam translation
        for (int i = 0; i != 3; ++i) {
            calibBin.midCamTranlation[i] = (float) cams[2].getTMatrix().at<double>(i, 0);
        }

        // mid cam dist coeffs
        for (int i = 0; i != 5; ++i) {
            calibBin.midCamDistCoeff[i] = (float) cams[2].getDistCoeffs().at<double>(i, 0);
        }

        // right cam image size
        calibBin.midCamImageSize[0] = (float) cams[2].getImageSize().width;
        calibBin.midCamImageSize[1] = (float) cams[2].getImageSize().height;

        return true;
    }


    bool B3D4AutoCalib::computeDispError(const cv::Mat& inputImgL, const cv::Mat& inputImgR,
        const b3di::CameraParams& inputCamL, const b3di::CameraParams& inputCamR,
        b3dd::DepthCameraType depthCamType, cv::Rect2f roiRect, float& dispError, int& validFeaturesCount)
    {
        TLog::setLogLevel(TLog::LOG_VERBOSE);
        CV_Assert(inputImgL.size() == inputImgR.size());
        CV_Assert(inputCamL.getImageSize() == inputCamR.getImageSize());
        CV_Assert(!inputImgL.empty());
        CV_Assert(inputCamL.getImageSize() != cv::Size());

        // resize input data for processing
        const float targetImageScale = 0.75f;
        const cv::Size targetImageSize{
            static_cast<int>(std::round(inputCamL.getImageSize().width * targetImageScale)),
            static_cast<int>(std::round(inputCamL.getImageSize().height * targetImageScale))
        };

        cv::Mat imgL, imgR;
        cv::resize(inputImgL, imgL, targetImageSize, 0, 0, INTER_AREA);
        cv::resize(inputImgR, imgR, targetImageSize, 0, 0, INTER_AREA);
        b3di::CameraParams camL = inputCamL, camR = inputCamR;
        camL.setImageSize(targetImageSize);
        camR.setImageSize(targetImageSize);

        Mat images[2] = { imgL, imgR };
        CameraParams cams[2] = { camL, camR };

        // compute disp roi
        b3dd::ROI computeDispRoi;
        if (roiRect != cv::Rect2f())
        {
            const int rows = images[0].rows;
            const int cols = images[0].cols;
            computeDispRoi = b3dd::ROI(
                static_cast<int>(std::round(cols * roiRect.x)),
                static_cast<int>(std::round(rows * roiRect.y)),
                static_cast<int>(std::round(cols * roiRect.width)),
                static_cast<int>(std::round(rows * roiRect.height))
            );
        }

        // Get extrinsic R,T relative to camL
        // We have only tested with the L cam at the origin
        // The code may not work if the L cam R,T is not identity
        Mat R, T;
        xform Mw1 = cams[1].getWorldToCameraXform();
        xform M0w = cams[0].getCameraToWorldXform();

        if (M0w != xform::identity()) {
            TLog::log(TLog::LOG_ERROR, "L cam extrinsic params must be identity");
            return false;
        }

        xform M01 = Mw1 * M0w;

        R = xformToMat3x3(M01);

        vec3 pt0 = M01 * trimesh::vec3(0, 0, 0);
        T = Mat(3, 1, CV_64FC1);
        T.at<double>(0) = pt0[0];
        T.at<double>(1) = pt0[1];
        T.at<double>(2) = pt0[2];

        // Search -maxCyOffset to +maxCyOffset to find the cy with the min disparity value
        // Move cy by 1 each time starting from the original position
        // stop when the disparity error is within a threshold
        Mat minR1, minQ, minDispMap;

        const double _MAX_ERROR = 1000;
        double minError = _MAX_ERROR;
        //	double dispError;
        b3dd::DepthProps depthProps = b3dd::DEPTH_PROPS[depthCamType];
        const int templSize = depthProps.stereoBlockSize / 2;
        const int searchSizeOffset = 3;
        const int searchSize = templSize + searchSizeOffset;

        vector<Point2f> minImgPts[2];
        vector<Point3f> minDispPts;

        const double ERR_THRESH = 1.0;
        float minOffset = ERR_THRESH;

        double featureRatio = 0;
        vector<vector<Point2f>> featurePoints;
        validFeaturesCount = 0;
        float yOffset = 0;
        CameraParams compCams[2];
        Mat rectMaps[2][2], R1, Q;
        Mat rectifiedImages[2], dispMap;

        vector<Point3f> thPoints;
        vector<Point2f> thImgPts[2];
        Vec2d dispOffset;

        // compute error with offset = 0
        {
            yOffset = 0;

            compCams[0] = cams[0];
            compCams[1] = cams[1];
            featurePoints.resize(2);

            // compute rectMaps
            computeRectifyMaps(compCams, rectMaps, R1, Q);

            // create cameraCalibExtPtr
            B3DCalibrationData calibBin;
            std::vector<b3di::CameraParams> camParams{ compCams[0], compCams[1],
                b3di::CameraParams() };
            bool ret = cvtCameraParamsToB3DCalibBin(camParams, calibBin);
            const B3DCalibrationData *calibBinPtr = &calibBin;
            b3dd::CameraCalibExtPtr cameraCalibExtPtr = std::make_shared<b3dd::CameraCalibExt>();
            cameraCalibExtPtr->createCalibrationData((unsigned char *)calibBinPtr,
                b3dd::DepthCameraType::DEPTHCAM_B3D4);

            // create depth processor
            b3dd::DepthProcessorExt depthProcessor;
            initDepthProcessor(cameraCalibExtPtr, depthProcessor, computeDispRoi);

            // compute rectified images and disp map
            b3dd::B3DImage imageL, imageR;
            cvMatToB3DImage(images[0], imageL);
            cvMatToB3DImage(images[1], imageR);
            b3dd::B3DImage rectL, rectR, disp;
            depthProcessor.computeRectsAndDisparityExt(imageL, imageR, rectL, rectR, disp);
            B3DImageToCvMat(rectL, rectifiedImages[0], b3dd::B3DImageType::B3DIMAGE_MONO);
            B3DImageToCvMat(rectR, rectifiedImages[1], b3dd::B3DImageType::B3DIMAGE_MONO);
            B3DImageToCvMat(disp, dispMap, b3dd::B3DImageType::B3DIMAGE_DISP_16);

            validFeaturesCount = geMatchingFeaturesFromDisparity(rectifiedImages[0], dispMap,
                featurePoints[0],
                featurePoints[1]);
            if (validFeaturesCount == 0) {
                TLog::log(TLog::LOG_WARNING,
                    "B3D4AutoCalib::computeDispError Insuffcient number of feature points.");
                return false;
            }
            else {
                featureRatio = (double)validFeaturesCount / featurePoints[0].size();

                dispError = getDisparityPointsFromFeatures(featurePoints[0], featurePoints[1],
                    rectifiedImages[0], rectifiedImages[1],
                    rectMaps[0][0], rectMaps[1][0],
                    templSize, searchSize, thPoints,
                    thImgPts[0], thImgPts[1], dispOffset);

                TLog::log(TLog::LOG_INFO, "B3D4AutoCalib::computeDispError Error[%.4f]", dispError);
            }
        }

        return true;
    }


    bool B3D4AutoCalib::recalibrate(const Mat &inputImgL, const Mat &inputImgR,
                                    CameraParams &inputCamL, CameraParams &inputCamR,
                                    bool &calibChanged, b3dd::DepthCameraType depthSDKCamType,
                                    int maxCyOffset, Rect2f roiRect, int depthCamType,
                                    float errorThr)
    {
        TLog::setLogLevel(TLog::LOG_VERBOSE);
        CV_Assert(inputImgL.size() == inputImgR.size());
        CV_Assert(inputCamL.getImageSize() == inputCamR.getImageSize());
        CV_Assert(!inputImgL.empty());
        CV_Assert(inputCamL.getImageSize() != cv::Size());

        // resize input data for processing
        const float targetImageScale = 0.75f;
        const cv::Size targetImageSize{
            static_cast<int>(std::round(inputCamL.getImageSize().width * targetImageScale)),
            static_cast<int>(std::round(inputCamL.getImageSize().height * targetImageScale))
        };

        cv::Mat imgL, imgR;
        cv::resize(inputImgL, imgL, targetImageSize, 0, 0, INTER_AREA);
        cv::resize(inputImgR, imgR, targetImageSize, 0, 0, INTER_AREA);
        b3di::CameraParams camL = inputCamL, camR = inputCamR;
        camL.setImageSize(targetImageSize);
        camR.setImageSize(targetImageSize);

        Mat images[2] = {imgL, imgR};
        CameraParams cams[2] = {camL, camR};

        TLog::log(TLog::LOG_VERBOSE,
                  "B3D4AutoCalib::recalibrate original fx=%.2f fy=%.2f cx=%.2f cy=%.2f",
                  inputCamR.fx(), inputCamR.fy(), inputCamR.cx(), inputCamR.cy());

        // compute disp roi
        b3dd::ROI computeDispRoi;
        if (roiRect != cv::Rect2f())
        {
            const int rows = images[0].rows;
            const int cols = images[0].cols;
            computeDispRoi = b3dd::ROI(
                static_cast<int>(std::round(cols * roiRect.x)),
                static_cast<int>(std::round(rows * roiRect.y)),
                static_cast<int>(std::round(cols * roiRect.width)),
                static_cast<int>(std::round(rows * roiRect.height))
            );
        }

        // Get extrinsic R,T relative to camL
        // We have only tested with the L cam at the origin
        // The code may not work if the L cam R,T is not identity
        Mat R, T;
        xform Mw1 = cams[1].getWorldToCameraXform();
        xform M0w = cams[0].getCameraToWorldXform();

        if (M0w != xform::identity()) {
            TLog::log(TLog::LOG_ERROR, "L cam extrinsic params must be identity");
            return false;
        }

        xform M01 = Mw1 * M0w;

        R = xformToMat3x3(M01);

        vec3 pt0 = M01 * trimesh::vec3(0, 0, 0);
        T = Mat(3, 1, CV_64FC1);
        T.at<double>(0) = pt0[0];
        T.at<double>(1) = pt0[1];
        T.at<double>(2) = pt0[2];


        // Search -maxCyOffset to +maxCyOffset to find the cy with the min disparity value
        // Move cy by 1 each time starting from the original position
        // stop when the disparity error is within a threshold
        Mat minR1, minQ, minDispMap;

        const double _MAX_ERROR = 1000;
        double minError = _MAX_ERROR;
        //	double dispError;
        b3dd::DepthProps depthProps = b3dd::DEPTH_PROPS[depthSDKCamType];
        const int templSize = depthProps.stereoBlockSize / 2;
        const int searchSize = maxCyOffset + templSize;

        vector<Point2f> minImgPts[2];
        vector<Point3f> minDispPts;

        const double ERR_THRESH = 1.0;
        float minOffset = ERR_THRESH;

        bool done = false;

        //if (maxCyOffset > 0) {
        int n = maxCyOffset * 2 + 1;
        int numThreads = n > 2 ? 2 : n;
        //	const int numThreads = 1;
        int i;
        double featureRatio = 0;
        vector<vector<Point2f>> featurePoints;
        int validFeaturesCount = 0;
        float yOffset = 0;
        CameraParams compCams[2];

        Mat rectMaps[2][2], R1, Q;

        Mat rectifiedImages[2], dispMap;

        vector<Point3f> thPoints;
        vector<Point2f> thImgPts[2];
        double dispError;
        Vec2d dispOffset;

#ifdef USE_OMP
        // Get the number of processors in this system
        int nProc = omp_get_num_procs();

        if (numThreads > nProc)
            numThreads = nProc;

#endif

        // compute error with offset = 0
        {
            yOffset = 0;

            compCams[0] = cams[0];
            compCams[1] = cams[1];
            featurePoints.resize(2);

            compCams[1].setIntrinsic(cams[1].fx(), cams[1].fy(), cams[1].cx(),
                cams[1].cy() + yOffset, cams[1].getImageSize());

            // compute rectMaps
            computeRectifyMaps(compCams, rectMaps, R1, Q);

            // create cameraCalibExtPtr
            B3DCalibrationData calibBin;
            std::vector<b3di::CameraParams> camParams{ compCams[0], compCams[1],
                b3di::CameraParams() };
            bool ret = cvtCameraParamsToB3DCalibBin(camParams, calibBin);
            const B3DCalibrationData *calibBinPtr = &calibBin;
            b3dd::CameraCalibExtPtr cameraCalibExtPtr = std::make_shared<b3dd::CameraCalibExt>();
            cameraCalibExtPtr->createCalibrationData((unsigned char *)calibBinPtr,
                b3dd::DepthCameraType::DEPTHCAM_B3D4);

            // create depth processor
            b3dd::DepthProcessorExt depthProcessor;
            initDepthProcessor(cameraCalibExtPtr, depthProcessor, computeDispRoi);

            // compute rectified images and disp map
            b3dd::B3DImage imageL, imageR;
            cvMatToB3DImage(images[0], imageL);
            cvMatToB3DImage(images[1], imageR);
            b3dd::B3DImage rectL, rectR, disp;
            depthProcessor.computeRectsAndDisparityExt(imageL, imageR, rectL, rectR, disp);
            B3DImageToCvMat(rectL, rectifiedImages[0], b3dd::B3DImageType::B3DIMAGE_MONO);
            B3DImageToCvMat(rectR, rectifiedImages[1], b3dd::B3DImageType::B3DIMAGE_MONO);
            B3DImageToCvMat(disp, dispMap, b3dd::B3DImageType::B3DIMAGE_DISP_16);

            validFeaturesCount = geMatchingFeaturesFromDisparity(rectifiedImages[0], dispMap,
                featurePoints[0],
                featurePoints[1]);
            if (validFeaturesCount == 0) {
                TLog::log(TLog::LOG_WARNING,
                    "B3D4AutoCalib::recalibrate cy=%.4f insuffcient number of feature points.",
                    compCams[1].cy());
            }
            else {
                featureRatio = (double)validFeaturesCount / featurePoints[0].size();

                dispError = getDisparityPointsFromFeatures(featurePoints[0], featurePoints[1],
                    rectifiedImages[0], rectifiedImages[1],
                    rectMaps[0][0], rectMaps[1][0],
                    templSize, searchSize, thPoints,
                    thImgPts[0], thImgPts[1], dispOffset);

                TLog::log(TLog::LOG_VERBOSE, "B3D4AutoCalib::recalibrate cy offset=%.1f (%.2f) disparity error=%.4f pt count=%d ratio=%.4f",
                    yOffset / targetImageScale, dispOffset[1], dispError, validFeaturesCount, featureRatio);
                
                // skip recalib if the err of offset=0 less than the err thr
                if (yOffset == 0 && dispError < errorThr)
                {
                    TLog::log(TLog::LOG_VERBOSE, "B3D4AutoCalib::recalibrate cy offset=%.1f err[%.2f] < ERROR_THR[%.2f], skip recalib",
                        yOffset / targetImageScale, dispError, errorThr);
                    calibChanged = false;
                    return true;
                }

                if (dispError < minError) {
                    minR1 = R1.clone();
                    minQ = Q.clone();
                    minDispMap = dispMap.clone();

                    minError = dispError;

                    // add the sub pixel offset
                    // don't do this if the dispOffset is greater than 1
                    minOffset = fabs(dispOffset[1]) < 1.0 ? (float)(yOffset - dispOffset[1])
                        : yOffset;

                    minImgPts[0] = thImgPts[0];
                    minImgPts[1] = thImgPts[1];
                    minDispPts = thPoints;

                    CV_Assert(minDispPts.size() == minImgPts[0].size());
                    CV_Assert(minDispPts.size() == minImgPts[1].size());
                }
            }
        }

#ifndef _DEBUG
#pragma omp parallel for schedule(dynamic, 1) private(i, yOffset, compCams, rectMaps, rectifiedImages, dispMap, thPoints, thImgPts, featureRatio, featurePoints, validFeaturesCount) num_threads(numThreads)
#endif // !_DEBUG
        for (i = 1; i < n; i++) {
#ifdef _SHOW_DBG
#ifdef USE_OMP
#pragma omp critical
            TLog::log(TLog::LOG_VERBOSE, "theadID=%d i=%d", omp_get_thread_num(), i);
#endif
#endif
            // search all in the range
            //if (done)
            //	continue;

            //yOffset = (float)-maxCyOffset + (float)i;

            // yOffset order: 0, -1, 1, -2, 2, -3, 3, ...
            if (i == 0)
                yOffset = 0;
            else {
                yOffset = (float) ((i + 1) / 2 * targetImageScale);
                if (i % 2)
                    yOffset = -yOffset;
            }

            compCams[0] = cams[0];
            compCams[1] = cams[1];
            featurePoints.resize(2);

            compCams[1].setIntrinsic(cams[1].fx(), cams[1].fy(), cams[1].cx(),
                                     cams[1].cy() + yOffset, cams[1].getImageSize());

            // compute rectMaps
            computeRectifyMaps(compCams, rectMaps, R1, Q);

            // create cameraCalibExtPtr
            B3DCalibrationData calibBin;
            std::vector<b3di::CameraParams> camParams{compCams[0], compCams[1],
                                                      b3di::CameraParams()};
            bool ret = cvtCameraParamsToB3DCalibBin(camParams, calibBin);
            const B3DCalibrationData *calibBinPtr = &calibBin;
            b3dd::CameraCalibExtPtr cameraCalibExtPtr = std::make_shared<b3dd::CameraCalibExt>();
            cameraCalibExtPtr->createCalibrationData((unsigned char *) calibBinPtr,
                                                     b3dd::DepthCameraType::DEPTHCAM_B3D4);

            // create depth processor
            b3dd::DepthProcessorExt depthProcessor;
            initDepthProcessor(cameraCalibExtPtr, depthProcessor, computeDispRoi);

            // compute rectified images and disp map
            b3dd::B3DImage imageL, imageR;
            cvMatToB3DImage(images[0], imageL);
            cvMatToB3DImage(images[1], imageR);
            b3dd::B3DImage rectL, rectR, disp;
            depthProcessor.computeRectsAndDisparityExt(imageL, imageR, rectL, rectR, disp);
            B3DImageToCvMat(rectL, rectifiedImages[0], b3dd::B3DImageType::B3DIMAGE_MONO);
            B3DImageToCvMat(rectR, rectifiedImages[1], b3dd::B3DImageType::B3DIMAGE_MONO);
            B3DImageToCvMat(disp, dispMap, b3dd::B3DImageType::B3DIMAGE_DISP_16);

            validFeaturesCount = geMatchingFeaturesFromDisparity(rectifiedImages[0], dispMap,
                                                                 featurePoints[0],
                                                                 featurePoints[1]);
            if (validFeaturesCount == 0) {
                TLog::log(TLog::LOG_WARNING,
                          "B3D4AutoCalib::recalibrate cy=%.4f insuffcient number of feature points.",
                          compCams[1].cy() / targetImageScale);
            } else {
                featureRatio = (double) validFeaturesCount / featurePoints[0].size();

                dispError = getDisparityPointsFromFeatures(featurePoints[0], featurePoints[1],
                                                           rectifiedImages[0], rectifiedImages[1],
                                                           rectMaps[0][0], rectMaps[1][0],
                                                           templSize, searchSize, thPoints,
                                                           thImgPts[0], thImgPts[1], dispOffset);
//#ifdef _SHOW_DBG
#pragma omp critical
                TLog::log(TLog::LOG_VERBOSE, "AutoCalib::recalibrate cy offset=%.1f (%.2f) disparity error=%.4f pt count=%d ratio=%.4f",
                    yOffset / targetImageScale, dispOffset[1], dispError, validFeaturesCount, featureRatio);
//#endif
                if (dispError < minError) {
#pragma omp critical
                    {
                        minR1 = R1.clone();
                        minQ = Q.clone();
                        minDispMap = dispMap.clone();

                        minError = dispError;

                        // add the sub pixel offset
                        // don't do this if the dispOffset is greater than 1
                        minOffset = fabs(dispOffset[1]) < 1.0 ? (float) (yOffset - dispOffset[1])
                                                              : yOffset;

                        minImgPts[0] = thImgPts[0];
                        minImgPts[1] = thImgPts[1];
                        minDispPts = thPoints;

                        CV_Assert(minDispPts.size() == minImgPts[0].size());
                        CV_Assert(minDispPts.size() == minImgPts[1].size());

                        //if (minError < ERR_THRESH)
                        //	done = true;
                    }
                }
            }
        }

        if (minDispPts.size() == 0 || minError == _MAX_ERROR) {
            TLog::log(TLog::LOG_ERROR, "AutoCalib::recalibration failed");
            return false;
        }

        // set the final cy value
        TLog::log(TLog::LOG_VERBOSE, "AutoCalib::recalibrate best offset is %.3f error=%.4f",
                  minOffset / targetImageScale, minError);

        CameraParams outputCams[2];
        outputCams[0] = cams[0];
        outputCams[1] = cams[1];
        double cy1 = cams[1].cy() + minOffset;
        //compCams[1].setIntrinsic(cams[1].fx(), cams[1].fy(), cams[1].cx(), cy1, cams[1].getImageSize());
        outputCams[1].setIntrinsic(cams[1].fx(), cams[1].fy(), cams[1].cx(), cy1,
                                   cams[1].getImageSize());

        //// This is to tell the caller that the calibration has changed significantly that the head pose estimates previously
        //// computed using the old calibration probably doesn't work any more
        //if (fabs(minOffset) < 0.5) {
        //    calibChanged = false;
        //    return true;    // skip recompute extrinsics
        //} else {
        //    calibChanged = true;
        //}

        // recomputed R cam position difference threshold
        // Ignore the recomputed R cam extrinisc if the difference threshold is exceeded
        const double R_POS_THRESH = 1.0;

        //		inputCamR = compCams[1];
        //inputCamR = outputCams[1];

        // Recomputing extrinsic sometimes make it worse so disable if the recomputed one is too different
        //double oldBaseline = fabs(compCams[1].getPosition()[0] - compCams[0].getPosition()[0]);

        // if the depth cam type is known, use the design camera parameters as old position
        // otherwise, use the input camera parameters as old position
        Vec3d oldPos = outputCams[1].getPosition();
        if (depthCamType < 0 || depthCamType >= NUM_DEPTHCAMS) {
            TLog::log(TLog::LOG_VERBOSE,
                      "AutoCalib::recalibrate Use input translation as R old pos");
        } else {
            const DepthCamProps depthCamProps = DEPTHCAM_PROPS[depthCamType];
            oldPos[0] = depthCamProps.stereoBaseline * oldPos[0] / std::fabs(oldPos[0]);
            TLog::log(TLog::LOG_VERBOSE,
                      "AutoCalib::recalibrate Use design R cam position as R old pos");
        }

        // cy has changed
        // recompute the extrinsic using the new cy value
        if (computeExtrinsicFormDisparityPoints(minDispPts, minImgPts, minR1, minQ, outputCams)) {
            // Use the 3d position difference instead of just comparing the baseline, which is prone to error
            //double newBaseline = fabs(compCams[1].getPosition()[0] - compCams[0].getPosition()[0]);
            Vec3d compPos = outputCams[1].getPosition();
            double posDiff = cv::norm(compPos - oldPos);

            // make sure the returned extrinsic params are reasonable
            bool computedExtrinsics = false;
            if (posDiff < R_POS_THRESH) {
                TLog::log(TLog::LOG_VERBOSE,
                    "AutoCalib::recalibrate recompute extrinsic params old R pos=%.2f %.2f %.2f new R pos=%.2f %.2f %.2f diff=%.2f",
                    oldPos[0], oldPos[1], oldPos[2], compPos[0], compPos[1], compPos[2],
                    posDiff);

                // Use the recomuted extrinsic
                inputCamR = outputCams[1];
                inputCamR.setImageSize(inputCamL.getImageSize());
                computedExtrinsics = true;
            }
            else
            {
                // compute extrinsics with subset of rand input pts
                int attemptCnt = 5;
                const float targetPtRatio = 0.8f;
                while (attemptCnt-- != 0
                    && computeExtrinsicFormRandDisparityPoints(minDispPts, minImgPts, minR1, minQ, outputCams, targetPtRatio))
                {
                    compPos = outputCams[1].getPosition();
                    posDiff = cv::norm(compPos - oldPos);

                    // make sure the returned extrinsic params are reasonable
                    if (posDiff < R_POS_THRESH)
                    {
                        TLog::log(TLog::LOG_VERBOSE,
                            "AutoCalib::recalibrate recompute extrinsic params old R pos=%.2f %.2f %.2f new R pos=%.2f %.2f %.2f diff=%.2f",
                            oldPos[0], oldPos[1], oldPos[2], compPos[0], compPos[1], compPos[2],
                            posDiff);

                        // Use the recomuted extrinsic
                        inputCamR = outputCams[1];
                        inputCamR.setImageSize(inputCamL.getImageSize());
                        computedExtrinsics = true;
                        break;
                    }
                }
            }

            // check if computed extrinsics
            if (computedExtrinsics)
            {
                calibChanged = true;
                TLog::log(TLog::LOG_VERBOSE,
                    "AutoCalib::recalibrate new fx=%.2f fy=%.2f cx=%.2f cy=%.2f error=%.4f",
                    inputCamR.fx(), inputCamR.fy(), inputCamR.cx(), inputCamR.cy(), minError);
            }
            else
            {
                calibChanged = false;
                TLog::log(TLog::LOG_VERBOSE,
                    "AutoCalib::recalibrate Ignore recomputed extrinsic params old R pos=%.2f %.2f %.2f new R pos=%.2f %.2f %.2f diff=%.2f",
                    oldPos[0], oldPos[1], oldPos[2], compPos[0], compPos[1], compPos[2],
                    posDiff);
            }

            return true;
        }
        else {
            TLog::log(TLog::LOG_ERROR,
                "AutoCalib::recalibrate Failed to recompute extrinsic params");
            return false;
        }

        return true;
    }
}