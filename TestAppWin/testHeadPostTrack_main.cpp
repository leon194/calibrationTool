#include <b3di/trackheadpose.h>
#include <b3di/FaceTrackerASM.h>
#include <b3di/B3d_utils.h>
#include <b3di/B3d_files.h>

#include <b3ddepth/utils/TLog.h>
#include <b3ddepth/core/B3DImage.h>
#include <b3ddepth/utils/B3DFileIO.h>
#include <b3ddepth/utils/imageutils.h>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "b3di/PointCloud.h"

using namespace std;
using namespace b3dd;
using namespace b3di;

enum CameraType {
    CAMERA_L = 0,  // IR left  camera (from camera view facing the world)
    CAMERA_R = 1,  // IR right camera
    CAMERA_M = 2,  // color    camera
    CAMERA_D = 3   // depth    camera
};

void B3DImageToCvMat(const B3DImage& inImage, cv::Mat& outMat) {

    B3DImageType type = inImage.type();
    int matType;

    switch (type) {
        case B3DIMAGE_MONO: matType = CV_8UC1;
            break;
        case B3DIMAGE_DEPTH_16: matType = CV_16UC1;
            break;
        case B3DIMAGE_RGB: matType = CV_8UC3;
            break;
        default:
            logError("Unsupported B3DImageType: %s ! ", type);
            return;
    }

    outMat = cv::Mat(inImage.rows(), inImage.cols(), matType);
    std::memcpy(outMat.data, inImage.data(), inImage.rowSize() * inImage.rows() * sizeof(unsigned char));
};

void loadB3DImages(const std::string filepath,
    std::vector<B3DImage>& imageContainer, std::vector<cv::String>& filenames) {

    std::string filepattern = "*.png";

    logInfo("Loading images from :  %s", filepath.c_str());

    imageContainer.clear();

    b3di::FileUtils::getDirFiles2(filepath + filepattern, filenames);

    for (auto filename : filenames) {

        B3DImage image;
        loadB3DImage(filepath + filename, image);

        if (image.isEmpty()) {
            logError("cannot load image : %s",filename.c_str());
            return;
        }
        else {
            imageContainer.push_back(image);
        }
    }
    return;
}

void loadB3DImagesJ(const std::string filepath,
    std::vector<B3DImage>& imageContainer, std::vector<cv::String>& filenames) {

    std::string filepattern = "*.jpg";

    logInfo("Loading images from :  %s", filepath.c_str());

    imageContainer.clear();

    b3di::FileUtils::getDirFiles2(filepath + filepattern, filenames);

    for (auto filename : filenames) {

        B3DImage image;
        loadB3DImage(filepath + filename, image);

        if (image.isEmpty()) {
            logError("cannot load image : %s", filename.c_str());
            return;
        }
        else {
            imageContainer.push_back(image);
        }
    }
    return;
}

void loadCalibrationData(vector<b3di::CameraParams>& outCamParams) {

    // Load Calibration Data to tmpCamParams
    vector<b3di::CameraParams> tmpCamParams;
    tmpCamParams.resize(b3di::MAX_NUM_CAMS);


    tmpCamParams[b3di::CAM_L_INDEX].loadParams("./CalibrationFiles/leftCam.yml");
    tmpCamParams[b3di::CAM_R_INDEX].loadParams("./CalibrationFiles/rightCam.yml");
    tmpCamParams[b3di::CAM_M_INDEX].loadParams("./CalibrationFiles/midCam.yml");
    tmpCamParams[b3di::CAM_M_INDEX].scaleImageSize(0.5);


    // Double check if calibration data loaded successfully
    outCamParams = tmpCamParams;
}

int initTracker(TrackHeadPose_Input &in, TrackHeadPose_Output &out, const cv::Mat &depthMap) {

    /* load landMarks */
    b3di::FaceTracker::FaceLandmarks mFaceLandmarks;
    std::string landMarkPath = "./faceM_track.yml";
    b3di::FaceTracker::readFile(landMarkPath, mFaceLandmarks);

    /* load camera params */
    std::vector<b3di::CameraParams> inputCams;
    loadCalibrationData(inputCams);

    // compute landmarks in color space if computeDepthSuccess
    if (!depthMap.empty()) {
        b3di::CameraParams depthCam =
            inputCams[b3di::CAM_L_INDEX];
        depthCam.setImageSize(depthMap.size());

        const b3di::CameraParams &colorCam =
            inputCams[b3di::CAM_M_INDEX];

        // compute depth in rgb space
        cv::Mat depthInColorSpace;
        b3di::PointCloud pc;
        pc.create(depthMap, depthCam.getIntrinsicMat());
        pc.transformPoints(colorCam.getWorldToCameraXform() * depthCam.getCameraToWorldXform());
        pc.projectPointsToDepth(
            colorCam.getIntrinsicMat(), colorCam.getDistCoeffs(),
            colorCam.getImageSize(), depthInColorSpace, 1.0, true);
        cv::Ptr<b3di::FaceTracker> ft;
        b3di::FaceTracker::FaceTrackerType landmarkType = b3di::FaceTracker::FT_ASM;
        ft = b3di::FaceTracker::newFaceTrackerPtr(mFaceLandmarks);
        // compute 3d landmarks
        ft->scaleLandmarks2d(colorCam.getImageSize());
        ft->get3dLandmarksFrom2d(depthInColorSpace, colorCam);
        ft->projectLandmarks(
            depthCam.getWorldToCameraXform() * colorCam.getCameraToWorldXform(),
            depthCam);
        mFaceLandmarks = ft->getFaceLandmarks();
    }
    else {
        cout << "input depth mat is empty" << endl;
    }

    in.inputCams = inputCams;
    in.show = false;
    in.maxICPError = (float)b3di::ICP_MAX_ERROR;
    in.computeHeadDepth = false;
    //in.asmTrackerDataPath = "../thirdparty/stasm4.1.0/data/";
    in.faceLandmarks = mFaceLandmarks;
    in.depthCameraType = b3di::DEPTHCAM_B3D4;

    auto depthCameraType = b3di::DepthCamType::DEPTHCAM_B3D4;
    b3di::DepthCamProps depthCamProps = b3di::DEPTHCAM_PROPS[depthCameraType];
    int trackingImageHeight = depthCamProps.trackingImageHeight;
    auto imageLSize = inputCams[b3di::CAM_L_INDEX].getImageSize();
    in.imageScale = (float)trackingImageHeight / imageLSize.height;
    in.depthCameraType = depthCameraType;

    out.headPoseContainerPtr = std::make_shared<b3di::CameraContainer>();
    out.headPoseContainerPtr->clear();

    int ret = -1;
    ret = initHeadPoseTracker(in, out);
    logVerbose("initHeadPoseTracker : %d", ret);

    return ret;
}

void main() {

    b3di::TLog::setLogLevel(b3di::TLog::LOG_ALL);
    std::vector<double> headRotation{ 0.0,0.0,0.0 };

    trimesh::xform headXf;
    float trackingError = -1;
    std::vector<b3di::CameraParams> _inputCams;

    vector<B3DImage> imagesL, imagesR, imagesM, imagesD;
    vector<cv::String> imageNamesL, imageNamesR, imageNamesM, imageNamesD;
#if 0
    loadB3DImages("./L/", imagesL, imageNamesL);
    loadB3DImages("./R/", imagesR, imageNamesR);
    loadB3DImagesJ("./M/", imagesM, imageNamesM);
#endif
    loadB3DImages("./L/", imagesL, imageNamesL);
    loadB3DImages("./D/", imagesD, imageNamesD);
    loadCalibrationData(_inputCams);
    
    cv::Mat cvFrameImages[b3di::MAX_NUM_CAMS];

    TrackHeadPose_Input  trackHeadInput;
    TrackHeadPose_Output trackHeadOutput;

    int ret = -1;
    std::string nPath = "D/" + imageNamesD[0];
    cv::Mat ID = cv::imread(nPath, cv::IMREAD_ANYDEPTH);
    ret = initTracker(trackHeadInput, trackHeadOutput, ID);
    cout << "initHeadPoseTracker : " << ret << endl;


    int trackret = -1;
    //cout << "imagesL size  : " << imagesL.size() << endl;
    for (int x = 0; x < imagesL.size(); x++) {
#if 0
        cv::Mat ImageL, ImageR;
        B3DImageToCvMat(imagesL[x], ImageL);
        B3DImageToCvMat(imagesR[x], ImageR);
        cvFrameImages[b3di::CAM_L_INDEX] = ImageL;
        cvFrameImages[b3di::CAM_R_INDEX] = ImageR;
#endif
        std::string path = "D/" + imageNamesD[x];
        cout << "path : " << path << endl;
        cv::Mat ImageD = cv::imread(path, cv::IMREAD_ANYDEPTH);
        cvFrameImages[b3di::CAM_D_INDEX] = ImageD;

        //if (x == 0) {
        //    cvFrameImages[b3di::CAM_M_INDEX] = cv::imread("M/M_001_2832320_00001.jpg");
        //}

        trackret = trackHeadPoseFrame(0, cvFrameImages, headXf, trackingError, trackHeadInput, trackHeadOutput);
        logError("trackret %d", trackret);
#if 0
        // only update the headRotation if trackHeadPoseFrame succeeds
        if (trackret == 0) {
            // Convert headXf into rotation and translation
            if (true) {

                // This function is more accurate for large X and Z rotation
                // Use it with 1.9+ version API
                cv::Mat rot = b3di::rot2euler(b3di::xformToMat3x3(headXf));
                headRotation[0] = rot.at<double>(0);
                headRotation[1] = rot.at<double>(1);
                headRotation[2] = rot.at<double>(2);

            }
            else {
                b3di::getRotationFromXform(headXf, headRotation[0], headRotation[1], headRotation[2]);
            }

            headRotation[0] = b3di::radToDegree(headRotation[0]);
            headRotation[1] = b3di::radToDegree(headRotation[1]);
            headRotation[2] = b3di::radToDegree(headRotation[2]);
            logError("Iterator %d headRotation %f ,%f, %f", x, headRotation[0], headRotation[1], headRotation[2]);
#else
        CameraParams cam = trackHeadOutput.headPoseContainerPtr->getData(x);
        // Get the xform from frame 0 to frame i
        if (cam.hasExtrinsic()) {
            double rotation[3];
            //xform M0_i = cam.getWorldToCameraXform();
            trimesh::xform M0_i = cam.getCameraToWorldXform();
            //getRotationFromXform(M0_i, rotation[0], rotation[1], rotation[2]);
            cv::Mat rot = rot2euler(xformToMat3x3(M0_i));
            rotation[0] = rot.at<double>(0);
            rotation[1] = rot.at<double>(1);
            rotation[2] = rot.at<double>(1);

            logError("frame %d rotation X:%.1f Y:%.1f Z:%.1f", x,
                radToDegree(rotation[0]),
                radToDegree(rotation[1]),
                radToDegree(rotation[2]));
#endif
        }
    }
}