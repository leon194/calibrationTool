#include <Windows.h>
#include <String>
#include <vector>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
#include <regex>
#include <filesystem>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "b3di/FileUtils.h"
#include "b3di/CameraParams.h"
#include <b3di/trackheadpose.h>
#include <b3di/FaceTrackerASM.h>
#include <b3di/B3d_utils.h>
#include <b3di/B3d_files.h>
#include "b3di/B3d_types.h"
#include "b3di/PointCloud.h"

// include b3d depth sdk libs
#include "b3ddepth/utils/ext/B3DCalibrationData.h"
#include "b3ddepth/core/ext/CameraCalibExt.h"
#include "b3ddepth/core/ext/DepthProcessorExt.h"
#include <b3ddepth/utils/TLog.h>
#include "b3ddepth/utils/TProfile.h"

#include <omp.h>
#include <map>

using namespace std;
using namespace b3dd;
using namespace cv;

const String FACE_LANDMARk_NAME = "Camera/faceM_track.yml";

int decodeMode_arg;
int debug_arg;
int loglevel_arg;
int framecount_arg;
String dir_arg;
b3dd::DepthProcessorExt _depthProcessor;
TrackHeadPose_Input  trackHeadInput;
TrackHeadPose_Output trackHeadOutput;

std::mutex mtx_initTracker;             // mutex for init head pose tracker
std::mutex mtx_runTracker;              // mutex for run head pose tracker

const static string DECODE_VERSION = "0.1.4";

enum DecoMode
{
  DECODEMODE_ALL = 1,
  DECODEMODE_COLOR,
  DECODEMODE_IRS
};

const std::vector<String> CALIB_YMLS{
    "leftCam.yml",
    "rightCam.yml",
    "midCam.yml"
};

enum CameraSensor {
    LEFT_CAM = 0,
    RIGHT_CAM = 1,
    MID_CAM = 2,
    CAM_NUM = 3
};

const std::vector<int> IMAGE_SIZE{
    2448,
    3264,
    800,
    1280
};

enum ImageSize {
    COLOR_WIDTH = 0,
    COLOR_HEIGHT = 1,
    IR_WIDTH = 2,
    IR_HEIGHT = 3
};

enum FolderIndex {
    FOLDER_ROOT = 0,
    FOLDER_CAM_M = 1,
    FOLDER_CAM_L = 2,
    FOLDER_CAM_R = 3,
    FOLDER_CAM_D = 4,
    FOLDER_CAM_P = 5,
    FOLDER_END = 6
};

#include <windows.h>
#include <Winbase.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

vector<String> get_all_files_names_within_folder(String folder)
{
    vector<String> names;
    String search_path = b3di::FileUtils::appendPath(folder, "*.*");
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                names.push_back(fd.cFileName);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
    return names;
}

char* readFileBytes(const char *name)
{
    ifstream fl(name, ios::in | ios::binary);
    fl.seekg(0, ios::end);
    size_t len = fl.tellg();
    if (len == 0) {
      logError("File %s is empty.", name);
      return NULL;
    }
    char *ret = new char[len];
    fl.seekg(0, ios::beg);
    fl.read(ret, len);
    fl.close();
    logVerbose("File lens : %d", len);
    return ret;
}

String getFilenamePrefix(String input) {

    std::size_t found = input.find('.');
    return input.substr(0, found);
}

void CvMatToB3DImage(const cv::Mat& inMat, B3DImage& outImage, const double imageScale = 1.0) {

    int type = inMat.type();
    B3DImageType outType;
    switch (type) {
        case CV_8UC1:
            outType = B3DIMAGE_MONO;
            break;
        case CV_16UC1:
            outType = B3DIMAGE_DEPTH_16;
            break;
        default:
            logWarning("Unsupported B3DImageType : %d !", type);
            break;
    }

    // Construct B3DImage
    cv::Mat tmpMat;
    cv::resize(inMat, tmpMat, cv::Size(), imageScale, imageScale, cv::INTER_AREA);
    outImage = B3DImage(tmpMat.rows, tmpMat.cols, outType);
    std::memcpy(outImage.data(), tmpMat.data, tmpMat.cols * tmpMat.rows * sizeof(unsigned char));
}

bool cvtCameraParamsToB3DCalibBin(
    const std::vector<b3di::CameraParams>& cams,
    B3DCalibrationData& calibBin
)
{
    if (cams.size() != 3)
    {
        logError("cvt calib bin -- invalid number of input cams [%d]", cams.size());
        return false;
    }

    /* for left cam */
    // left cam intrinsic
    calibBin.leftCamIntrinsic[0] = (float)cams[0].fx();
    calibBin.leftCamIntrinsic[1] = (float)cams[0].fy();
    calibBin.leftCamIntrinsic[2] = (float)cams[0].cx();
    calibBin.leftCamIntrinsic[3] = (float)cams[0].cy();

    // left cam dist coeffs
    for (int i = 0; i != 5; ++i)
    {
        calibBin.leftCamDistCoeff[i] = (float)cams[0].getDistCoeffs().at<double>(i, 0);
    }

    // left cam image size
    calibBin.leftCamImageSize[0] = (float)cams[0].getImageSize().width;
    calibBin.leftCamImageSize[1] = (float)cams[0].getImageSize().height;

    /* for right cam */
    // right cam instrinsic
    calibBin.rightCamIntrinsic[0] = (float)cams[1].fx();
    calibBin.rightCamIntrinsic[1] = (float)cams[1].fy();
    calibBin.rightCamIntrinsic[2] = (float)cams[1].cx();
    calibBin.rightCamIntrinsic[3] = (float)cams[1].cy();

    // right cam dist coeffs
    for (int i = 0; i != 5; ++i)
    {
        calibBin.rightCamDistCoeff[i] = (float)cams[1].getDistCoeffs().at<double>(i, 0);
    }

    // right cam rotation
    for (int i = 0; i != 9; ++i)
    {
        calibBin.rightCamRotation[i] = (float)cams[1].getRMatrix().at<double>(i / 3, i % 3);
    }

    // right cam translation
    for (int i = 0; i != 3; ++i)
    {
        calibBin.rightCamTranlation[i] = (float)cams[1].getTMatrix().at<double>(i, 0);
    }

    // right cam image size
    calibBin.rightCamImageSize[0] = (float)cams[1].getImageSize().width;
    calibBin.rightCamImageSize[1] = (float)cams[1].getImageSize().height;

    /* for mid cam */
    // mid cam intrisics
    calibBin.midCamIntrinsic[0] = (float)cams[2].fx();
    calibBin.midCamIntrinsic[1] = (float)cams[2].fy();
    calibBin.midCamIntrinsic[2] = (float)cams[2].cx();
    calibBin.midCamIntrinsic[3] = (float)cams[2].cy();

    // mid cam rotation
    for (int i = 0; i != 9; ++i)
    {
        calibBin.midCamRotation[i] = (float)cams[2].getRMatrix().at<double>(i / 3, i % 3);
    }

    // mid cam translation
    for (int i = 0; i != 3; ++i)
    {
        calibBin.midCamTranlation[i] = (float)cams[2].getTMatrix().at<double>(i, 0);
    }

    // mid cam dist coeffs
    for (int i = 0; i != 5; ++i)
    {
        calibBin.midCamDistCoeff[i] = (float)cams[2].getDistCoeffs().at<double>(i, 0);
    }

    // right cam image size
    calibBin.midCamImageSize[0] = (float)cams[2].getImageSize().width;
    calibBin.midCamImageSize[1] = (float)cams[2].getImageSize().height;

    return true;
}

bool exportB3DRectifyMaps(
    const String& outputDir,
    const String& fileName,
    const B3DCalibrationData& calibBin
)
{
    const B3DCalibrationData* calibBinPtr = &calibBin;
    b3dd::CameraCalibPtr cameraCalibPtr = std::make_shared<b3dd::CameraCalib>();
    return cameraCalibPtr->cacheCalibrationData((unsigned char*)calibBinPtr, b3di::FileUtils::appendPath(outputDir, fileName));
}

cv::Mat computeDepth(b3dd::DepthProcessorExt& depthProcessor, cv::Mat cvMatL, cv::Mat cvMatR) {

    cv::Mat depthMap;
    const double imageScale = 0.75f;

    b3dd::B3DImage imageL, imageR;

    //resize input image
    CvMatToB3DImage(cvMatL, imageL, imageScale);
    CvMatToB3DImage(cvMatR, imageR, imageScale);

    // compute depth
    b3dd::B3DImage outDepthMap;
    bool computeDepthSuccess = false;
    computeDepthSuccess = depthProcessor.computeDepth(imageL, imageR, outDepthMap);

    if (!computeDepthSuccess)
    {
        logError("compute depth error !!");
        return depthMap;
    }

    // convert depth to cv::Mat
    depthMap = cv::Mat(outDepthMap.rows(), outDepthMap.cols(), CV_16UC1);
    std::memcpy(depthMap.data, outDepthMap.data(),
        outDepthMap.rowSize() * outDepthMap.rows() * sizeof(unsigned char));

    return depthMap;
}

int checkCalibData() {
    B3DCalibrationData calibBin;
    const String calibBinPath = b3di::FileUtils::appendPath(dir_arg, "CalibrationFiles");
    const String calibBinFileName = "b3dCalibData.bin";
    const String calibDataPath = b3di::FileUtils::appendPath(dir_arg, "calib.data");

    logVerbose("calibBinPath : %s, calibDataPath : %s", calibBinPath.c_str(), calibDataPath.c_str());

    if (!readB3DCalibrationData(calibBinPath, calibBinFileName, calibBin)) {
        logError("Fail to load calib bin file : %s", (b3di::FileUtils::appendPath(calibBinPath, calibBinFileName)).c_str());
        return -1;
    }

    const B3DCalibrationData* calibBinPtr = &calibBin;
    CameraCalibExtPtr _cameraCalibExtPtr = std::shared_ptr<CameraCalibExt>(new CameraCalibExt());

    _cameraCalibExtPtr->cacheCalibrationData(
        (unsigned char*)calibBinPtr,
        calibDataPath,
        DepthCameraType::DEPTHCAM_B3D4
    );

    /* generate calib.data half */
    bool ret = false;
    std::vector<b3di::CameraParams> camParams(CAM_NUM);
    cv::Size imageSize(IMAGE_SIZE[IR_WIDTH] * 0.75, IMAGE_SIZE[IR_HEIGHT] * 0.75);

    for (int x = 0; x < CAM_NUM; x++) {
        String filePath = b3di::FileUtils::appendPath(dir_arg, b3di::FileUtils::appendPath("CalibrationFiles", CALIB_YMLS[x]));
        ret = camParams[x].loadParams(filePath);
        if (!ret) {
            logError("can't load yml : %s", filePath.c_str());
            return -1;
        }
        if (x == LEFT_CAM || x == RIGHT_CAM)
            camParams[x].setImageSize(imageSize);
    }

    B3DCalibrationData calibBinHalf;
    ret = cvtCameraParamsToB3DCalibBin(camParams, calibBinHalf);

    if (!ret) {
        logError("can't convert to calibration.bin");
        return -1;
    }


    // export calab.data
    ret = exportB3DRectifyMaps(
        dir_arg,
        "/calib.data",
        calibBin
    );

    if (!ret) {
        logError("can't export calib.data");
        return -1;
    }

    return 0;
}

const String currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

void runOfflineDebug(const vector<String> folderNames) {
    vector<String> filenames = get_all_files_names_within_folder(dir_arg);

    const int bytesAvailable_M = IMAGE_SIZE[COLOR_WIDTH] * IMAGE_SIZE[COLOR_HEIGHT] * 3 / 2;
    const size_t bytesAvailable = IMAGE_SIZE[IR_WIDTH] * IMAGE_SIZE[IR_HEIGHT];


    std::vector<pair<String, pair<cv::Mat, cv::Mat>> > depthstdMaps;

    double decodeStart, decodeEnd, computeDepthStart, computeDepthEnd;

    logVerbose("get %d files.", filenames.size());
    String decodString = (DecoMode)decodeMode_arg == DECODEMODE_COLOR ? "decodeColor" :
        ((DecoMode)decodeMode_arg == DECODEMODE_IRS ? "decodeIRS" : "decodeAll");

    logAlways(",%s,%sStart", currentDateTime().c_str(), decodString.c_str());

    decodeStart = now_ms();
    String filePathMDummy;

#ifndef _DEBUG
#pragma omp parallel for
#endif // !_DEBUG
    for (int x = 0; x < filenames.size(); x++) {

        String filepath;
        String targetpath;

        targetpath = b3di::FileUtils::appendPath(dir_arg, filenames[x]);
        logVerbose("decode file %d, path : %s.", x, targetpath.c_str());

        unsigned char *buffer = (unsigned char*)readFileBytes(targetpath.c_str());

        if ((DecoMode)decodeMode_arg == DECODEMODE_COLOR || (DecoMode)decodeMode_arg == DECODEMODE_ALL) {
            cv::Mat frameYUV = cv::Mat::zeros(IMAGE_SIZE[COLOR_HEIGHT] * 3 / 2, IMAGE_SIZE[COLOR_WIDTH], CV_8UC1);
            cv::Mat frameRGB;

            memcpy(frameYUV.data, (unsigned char *)buffer,
                bytesAvailable_M * sizeof(unsigned char));
            cv::cvtColor(frameYUV, frameRGB, CV_YUV2BGR_NV21);

            filepath = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_M], "M_" + getFilenamePrefix(filenames[x]) + ".jpg");
            logVerbose("write to : %s.", filepath.c_str());
            cv::imwrite(filepath, frameRGB);

            buffer += bytesAvailable_M;
        }

        if ((DecoMode)decodeMode_arg == DECODEMODE_IRS || (DecoMode)decodeMode_arg == DECODEMODE_ALL) {
            cv::Mat frameL = cv::Mat::zeros(IMAGE_SIZE[IR_HEIGHT], IMAGE_SIZE[IR_WIDTH], CV_8UC1);
            cv::Mat frameR = cv::Mat::zeros(IMAGE_SIZE[IR_HEIGHT], IMAGE_SIZE[IR_WIDTH], CV_8UC1);


            memcpy(frameL.data, buffer, IMAGE_SIZE[IR_WIDTH] * IMAGE_SIZE[IR_HEIGHT] * sizeof(unsigned char));

            filepath = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_L], "L_" + getFilenamePrefix(filenames[x]) + ".png");
            logVerbose("write to : %s.", filepath.c_str());

            cv::imwrite(filepath, frameL);

            buffer += bytesAvailable;
            memcpy(frameR.data, buffer, IMAGE_SIZE[IR_WIDTH] * IMAGE_SIZE[IR_HEIGHT] * sizeof(unsigned char));

            filepath = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_R], "R_" + getFilenamePrefix(filenames[x]) + ".png");
            logVerbose("write to : %s.", filepath.c_str());
            cv::imwrite(filepath, frameR);

            /* prepare for depth path */
            filepath = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_D], "D_" + getFilenamePrefix(filenames[x]) + ".png");
            filePathMDummy = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_M], "M_" + getFilenamePrefix(filenames[x]) + ".jpg");
            depthstdMaps.push_back(std::make_pair(filepath, std::make_pair(frameL, frameR)));
        }
    }

    decodeEnd = now_ms();

    logAlways(",%s,%sFinish,%f sec", currentDateTime().c_str(), decodString.c_str(), (decodeEnd - decodeStart) / 1000);

    if ((DecoMode)decodeMode_arg == DECODEMODE_IRS || (DecoMode)decodeMode_arg == DECODEMODE_ALL) {
        logAlways(",%s,computeDepthStart", currentDateTime().c_str());
        computeDepthStart = now_ms();
        b3dd::DepthProcessorExt depthProcessor;

        // should only load calib.data and config depth processor for once
        // read calib.data from disk
        b3dd::CameraCalibExtPtr cameraCalibPtr = std::make_shared<b3dd::CameraCalibExt>();
        if (!cameraCalibPtr->loadFromFile(b3di::FileUtils::appendPath(dir_arg, "../calib.data"))) {
            logWarning("load calib.data failed");
        }
        else
        {
            /* Configure DepthProcessor settings */
            //b3dd::DepthProcessorExt depthProcessor;

            // load calib data
            depthProcessor.loadCalibrationData(cameraCalibPtr);

            // config depth processor
            b3dd::DepthConfigExtPtr depthConfigPtr =
                std::shared_ptr<b3dd::DepthConfigExt>(new b3dd::DepthConfigExt());
            depthConfigPtr->sceneMode = b3dd::SCENE_MODE::NORMAL;
            depthConfigPtr->optimizeMode = b3dd::OPTIMIZE_MODE::OPTIMIZE_HALF_RES;
            depthConfigPtr->depthRegistration = b3dd::DEPTH_REGISTRATION::REGISTER_L;
            depthConfigPtr->depthScale = 0.75f;
            depthConfigPtr->depthUnit = 0.02f;
            depthConfigPtr->useTwoDispMaps = true;
            depthConfigPtr->reduceDispBias = true;
            depthConfigPtr->useIRThres = false;

            depthProcessor.updateProcessorSettingsExt(depthConfigPtr);

#ifndef _DEBUG
#pragma omp parallel for
#endif // !_DEBUG
            for (int x = 0; x < depthstdMaps.size(); x++) {

                String filepath = depthstdMaps[x].first;
                cv::Mat depth = computeDepth(depthProcessor, depthstdMaps[x].second.first, depthstdMaps[x].second.second);

                logVerbose("write to : %s.", filepath.c_str());

                cv::imwrite(filepath, depth);
                cv::Mat dummyM = cv::Mat(1, 1, CV_8UC3);
                //cv::imwrite(filePathMDummy, dummyM);
            }
            computeDepthEnd = now_ms();
            logAlways(",%s,computeDepthFinish,%f sec", currentDateTime().c_str(), (computeDepthEnd - computeDepthStart) / 1000);
            depthProcessor.printTimingStatistics();
        }
    }
}

void loadCalibrationData(vector<b3di::CameraParams>& outCamParams) {

    // Load Calibration Data to tmpCamParams
    vector<b3di::CameraParams> tmpCamParams;
    tmpCamParams.resize(b3di::MAX_NUM_CAMS);

    tmpCamParams[b3di::CAM_L_INDEX].loadParams(b3di::FileUtils::appendPath(dir_arg, "CalibrationFiles/leftCam.yml"));
    tmpCamParams[b3di::CAM_R_INDEX].loadParams(b3di::FileUtils::appendPath(dir_arg, "CalibrationFiles/rightCam.yml"));
    tmpCamParams[b3di::CAM_M_INDEX].loadParams(b3di::FileUtils::appendPath(dir_arg, "CalibrationFiles/midCam.yml"));
    tmpCamParams[b3di::CAM_M_INDEX].scaleImageSize(0.5);

    // Double check if calibration data loaded successfully
    outCamParams = tmpCamParams;
}

void initTracker(TrackHeadPose_Input& in, TrackHeadPose_Output& out, const cv::Mat& depthMap) 
{
    // critical section (exclusive access to init tracker)
    mtx_initTracker.lock();

    if (in.depthCameraType == b3di::DepthCamType::DEPTHCAM_B3D4)
        return;

    /* load landMarks */
    b3di::FaceTracker::FaceLandmarks mFaceLandmarks;
    String landMarkPath = b3di::FileUtils::appendPath(dir_arg, FACE_LANDMARk_NAME);
    b3di::FaceTracker::readFile(landMarkPath, mFaceLandmarks);

    /* load camera params */
    std::vector<b3di::CameraParams> inputCams;
    loadCalibrationData(inputCams);

    // compute landmarks in color space if computeDepthSuccess
    if (!depthMap.empty())
    {
        b3di::CameraParams depthCam =
            inputCams[b3di::CAM_L_INDEX];
        depthCam.setImageSize(depthMap.size());

        const b3di::CameraParams& colorCam =
            inputCams[b3di::CAM_M_INDEX];

        // compute depth in rgb space
        cv::Mat depthInColorSpace;
        b3di::PointCloud pc;
        pc.create(depthMap, depthCam.getIntrinsicMat());
        pc.transformPoints(colorCam.getWorldToCameraXform()
            * depthCam.getCameraToWorldXform());
        pc.projectPointsToDepth(
            colorCam.getIntrinsicMat(), colorCam.getDistCoeffs(),
            colorCam.getImageSize(), depthInColorSpace, 1.0, true
        );

        cv::Ptr<b3di::FaceTracker> ft;
        b3di::FaceTracker::FaceTrackerType landmarkType = b3di::FaceTracker::FT_ASM;
        ft = b3di::FaceTracker::newFaceTrackerPtr(mFaceLandmarks);

        // compute 3d landmarks
        ft->scaleLandmarks2d(colorCam.getImageSize());
        ft->get3dLandmarksFrom2d(depthInColorSpace, colorCam);
        ft->projectLandmarks(
            depthCam.getWorldToCameraXform() * colorCam.getCameraToWorldXform(),
            depthCam
        );
        mFaceLandmarks = ft->getFaceLandmarks();
    }
    
    in.inputCams = inputCams;
    in.show = false;
    in.maxICPError = (float)b3di::ICP_MAX_ERROR;
    in.computeHeadDepth = false;
    //in.asmTrackerDataPath = "../thirdparty/stasm4.1.0/data/";
    in.faceLandmarks = mFaceLandmarks;

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
    logInfo("initHeadPoseTracker : %d",ret);

    mtx_initTracker.unlock();

    return;
}

int initDepthProcessor(b3dd::DepthProcessorExt& depthProcessor, const double imageScale = 0.75f) {

    /* check calib.data */
    checkCalibData();

    /* read calib.data from disk */
    b3dd::CameraCalibExtPtr cameraCalibPtr = std::make_shared<b3dd::CameraCalibExt>();
    if (!cameraCalibPtr->loadFromFile(b3di::FileUtils::appendPath(dir_arg, "calib.data"))) {
        logWarning("load calib.data failed");
        return -1;
    }

    depthProcessor.loadCalibrationData(cameraCalibPtr);

    /* config depth processor */
    b3dd::DepthConfigExtPtr depthConfigPtr =
        std::shared_ptr<b3dd::DepthConfigExt>(new b3dd::DepthConfigExt());
    depthConfigPtr->sceneMode = b3dd::SCENE_MODE::NORMAL;
    depthConfigPtr->optimizeMode = b3dd::OPTIMIZE_MODE::OPTIMIZE_HALF_RES;
    depthConfigPtr->depthRegistration = b3dd::DEPTH_REGISTRATION::REGISTER_L;
    depthConfigPtr->depthScale = imageScale;
    depthConfigPtr->depthUnit = 0.02f;
    depthConfigPtr->useTwoDispMaps = true;
    depthConfigPtr->reduceDispBias = true;
    depthConfigPtr->useIRThres = false;

    depthProcessor.updateProcessorSettingsExt(depthConfigPtr);
}

static bool writeSelectedFrames(const String& fileName, const std::vector<int>& selectedFrameIndices) {
  FileStorage fs(fileName, FileStorage::WRITE);
  if (fs.isOpened()) {
    fs << "selectedFrameIndices" << selectedFrameIndices;
    return true;
  }
  return false;
}

void processFile(const String& inputFileName, const String& inputDir, const std::vector<String>& folderNames) {

  logInfo("%s",inputFileName.c_str());
  auto targetpath = b3di::FileUtils::appendPath(inputDir, inputFileName);

  // read the file
  const size_t bytesAvailable = IMAGE_SIZE[IR_WIDTH] * IMAGE_SIZE[IR_HEIGHT];
  unsigned char *buffer = (unsigned char*)readFileBytes(targetpath.c_str());
  if (buffer == NULL) {
    logError("readFileBytes failed.");
    return;
  }
  cv::Mat frameL = cv::Mat::zeros(IMAGE_SIZE[IR_HEIGHT], IMAGE_SIZE[IR_WIDTH], CV_8UC1);
  cv::Mat frameR = cv::Mat::zeros(IMAGE_SIZE[IR_HEIGHT], IMAGE_SIZE[IR_WIDTH], CV_8UC1);

  // process raw data
  /* Frame L */
  memcpy(frameL.data, buffer, IMAGE_SIZE[IR_WIDTH] * IMAGE_SIZE[IR_HEIGHT] * sizeof(unsigned char));
  logInfo("");

  /* get file name from queue */
  String filenamePrefix;
  String filenameIndex;
  String filepath;

  std::size_t found = inputFileName.find_last_of(".");
  filenamePrefix = inputFileName.substr(0, found);
  found = inputFileName.find_first_of("_");
  filenameIndex = inputFileName.substr(0, found);

  filepath = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_L], "L_" + filenamePrefix + ".png");
  logInfo("Frame L: %s",filepath.c_str());

  if (debug_arg == 1300) {
    logVerbose("write to : %s.", filepath.c_str());
    cv::imwrite(filepath, frameL);
  }

  /* Frame R */
  buffer += bytesAvailable;
  memcpy(frameR.data, buffer, IMAGE_SIZE[IR_WIDTH] * IMAGE_SIZE[IR_HEIGHT] * sizeof(unsigned char));
  filepath = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_R], "R_" + filenamePrefix + ".png");
  logVerbose("Frame R: %s.", filepath.c_str());
  if (debug_arg == 1300) {
    cv::imwrite(filepath, frameR);
  }

  /* compute depth */
  b3dd::B3DImage outDepthMap;
  b3dd::B3DImage imageL, imageR;
  cv::Mat depthMap;

  //resize input image
  const double imageScale = 0.75f;
  CvMatToB3DImage(frameL, imageL, imageScale);
  CvMatToB3DImage(frameR, imageR, imageScale);

  const auto computeDepthSuccess = _depthProcessor.computeDepth(imageL, imageR, outDepthMap);
  if (!computeDepthSuccess)
  {
    logError("compute depth error: error code %d", computeDepthSuccess);
  }

  // convert depth to cv::Mat
  depthMap = cv::Mat(outDepthMap.rows(), outDepthMap.cols(), CV_16UC1);
  std::memcpy(depthMap.data, outDepthMap.data(),
    outDepthMap.rowSize() * outDepthMap.rows() * sizeof(unsigned char));

  const auto depthPrefix = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_D], "D_" + filenamePrefix);
  auto oldName = depthPrefix + "_tmp.png";
  auto newName = depthPrefix + ".png";
  auto result = cv::imwrite(oldName, depthMap);
  if (result)
    logVerbose("depth image file successfully written: %s", oldName.c_str());
  else
    logError("Error writing file: %s", oldName.c_str()); 
  
  result = rename(oldName.c_str(), newName.c_str());
  if (result == 0)
    logInfo("File successfully renamed: %s", newName.c_str());
  else
    logError("Error renaming file: %s", oldName.c_str());

  cv::String filePathMDummy = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_M], "M_" + filenamePrefix + ".jpg");
  cv::Mat dummyM = cv::Mat(1, 1, CV_8UC3);
  cv::imwrite(filePathMDummy, dummyM);
  logVerbose("write to : %s.", filePathMDummy.c_str());

  /* compute poseL*/
  int trackret = -1;
  cv::Mat cvFrameImages[b3di::MAX_NUM_CAMS];
  trimesh::xform headXf;
  float trackingError = -1;


  cvFrameImages[b3di::CAM_D_INDEX] = depthMap;

  mtx_runTracker.lock();
  // check if need to call initTracker
  if (trackHeadInput.depthCameraType != b3di::DepthCamType::DEPTHCAM_B3D4)
      initTracker(trackHeadInput, trackHeadOutput, depthMap);

  trackret = trackHeadPoseFrame(0, cvFrameImages, headXf, trackingError, trackHeadInput, trackHeadOutput);
  logVerbose("trackret %d", trackret);
  mtx_runTracker.unlock();

  stringstream ss;
  ss << setw(4) << setfill('0') << filenameIndex;
  auto ymlPrefix = b3di::FileUtils::appendPath(folderNames[FolderIndex::FOLDER_CAM_P], "CAM_" + ss.str());
  oldName = ymlPrefix + "_tmp.yml";
  newName = ymlPrefix + ".yml";
  auto camParam = trackHeadOutput.headPoseContainerPtr->getData(atoi(filenameIndex.c_str()) - 1);
  if (camParam.hasExtrinsic()) {
    double rotation[3];
    trimesh::xform M0_i = camParam.getCameraToWorldXform();

    Mat rot = b3di::rot2euler(b3di::xformToMat3x3(M0_i));
    rotation[0] = rot.at<double>(0);
    rotation[1] = rot.at<double>(1);
    rotation[2] = rot.at<double>(2);

    logVerbose("computeDepth: frame rotation X:%.1f Y:%.1f Z:%.1f",
      b3di::radToDegree(rotation[0]),
      b3di::radToDegree(rotation[1]),
      b3di::radToDegree(rotation[2]));
  }
  else {
    logError("Error camParam doesn't have intrin");
  }

  result = camParam.writeParams(oldName);
  if (result)
    logVerbose("headpose file successfully written: %s", oldName.c_str());
  else
    logError("Error writing file: %s", oldName.c_str()); 
  result = rename(oldName.c_str(), newName.c_str());
  if (result == 0)
    logVerbose("File successfully renamed: %s", newName.c_str());
  else
    logError("Error renaming file to: %s", newName.c_str());
}

void processFileAsync(const String& inputFileName, const String& inputDir, const std::vector<String>& folderNames) {

  //// https://stackoverflow.com/questions/23454793/whats-the-c-11-way-to-fire-off-an-asynchronous-task-and-forget-about-it
  //std::thread([]() { processFile(inputFileName); }).detach();

  logInfo("start processFileAsync: %s", inputFileName.c_str());
  // http://www.cplusplus.com/reference/thread/thread/detach/
  std::thread(processFile, inputFileName, inputDir, folderNames).detach();
}

// if name matches pattern return true
bool matchNamePattern(String name, String pattern) {
  return std::regex_match(name.c_str(), std::regex(pattern.c_str()));
}

// look for the next file in dirPath given the current fileName 
// update the currFileIndex if a file is found and return true
bool lookForNextFile(int& currFileIndex, const String& dirPath, String& nextFileNamePattern, String& nextFileName) {
  auto fileNames = get_all_files_names_within_folder(dirPath);

  for (auto& p : fileNames) {
    if (matchNamePattern(p, nextFileNamePattern)) {
      if (b3di::FileUtils::fileExists(b3di::FileUtils::appendPath(dirPath, p))) {
        std::cout << "---------------" << p << '\n';
        nextFileName = p;
        currFileIndex += 1;
        return true;
      }
      else {
        logWarning("Raw file %s doesn't exist yet.", p.c_str());
      }
    }
  }
  return false;
}

int main(int argc, const char* argv[])
{

    b3dd::TLog::setLogLevel(TLog::LOG_INFO);
    logInfo(">>>>>>>> decode version : %s <<<<<<<<", DECODE_VERSION.c_str());
    const cv::String keys =
        "{help h       |       | print this message              }"
        "{@dir         |       | path to the data directory      }"
        "{decodeMode m |   1   | 1 : all, 2 : color, 3 : IRS     }"
        "{debug      d |   0   | 1 : offline , 2 : realtime, 1300 : save L/R }"
        "{frames     f |  100  | frames to decode                }"
        "{loglevel   l |   2   | 2 : INFO , 1 : VERBOSE, 0 : ALL }"
        ;

    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }
    else {
        dir_arg = parser.get<String>("@dir");
        decodeMode_arg = parser.get<int>("decodeMode");
        debug_arg = parser.get<int>("debug");
        loglevel_arg = parser.get<int>("loglevel");
        framecount_arg = parser.get<int>("frames");
    }

    logAlways("Log level is: %d", loglevel_arg);
    b3dd::TLog::setLogLevel((b3dd::TLog::LogType)loglevel_arg);

    vector<String> folderNames;
    vector<String> folderNames2;
    folderNames.reserve(FolderIndex::FOLDER_END);
    folderNames2.reserve(FolderIndex::FOLDER_END);

    folderNames.push_back(b3di::FileUtils::appendPath(dir_arg, "Camera"));
    folderNames.push_back(b3di::FileUtils::appendPath(dir_arg, "Camera/M"));
    folderNames.push_back(b3di::FileUtils::appendPath(dir_arg, "Camera/L"));
    folderNames.push_back(b3di::FileUtils::appendPath(dir_arg, "Camera/R"));
    folderNames.push_back(b3di::FileUtils::appendPath(dir_arg, "Camera/D"));
    folderNames.push_back(b3di::FileUtils::appendPath(dir_arg, "Camera/poseL"));

    folderNames2.push_back(b3di::FileUtils::appendPath(dir_arg, "../Camera"));
    folderNames2.push_back(b3di::FileUtils::appendPath(dir_arg, "../Camera/M"));
    folderNames2.push_back(b3di::FileUtils::appendPath(dir_arg, "../Camera/L"));
    folderNames2.push_back(b3di::FileUtils::appendPath(dir_arg, "../Camera/R"));
    folderNames2.push_back(b3di::FileUtils::appendPath(dir_arg, "../Camera/D"));
    folderNames2.push_back(b3di::FileUtils::appendPath(dir_arg, "../Camera/poseL"));

    if (debug_arg == 1) {
        /* prepare folders */
        for (int x = 0; x < folderNames2.size(); x++) {
            if (!b3di::FileUtils::directoryExists(folderNames2[x]))
                b3di::FileUtils::createDir(folderNames2[x]);
        }
        runOfflineDebug(folderNames2);
    }
    else {
        /* prepare folders */
        for (int x = 0; x < folderNames.size(); x++) {
            if (!b3di::FileUtils::directoryExists(folderNames[x]))
                b3di::FileUtils::createDir(folderNames[x]);
        }

        String folder_to_monitor = b3di::FileUtils::appendPath(dir_arg, "stereos");

        LPTSTR path = new TCHAR[folder_to_monitor.size() + 1];
        strcpy(path, folder_to_monitor.c_str());

        /* check if facelandmark file exist first */
        while (!b3di::FileUtils::fileExists(b3di::FileUtils::appendPath(dir_arg, FACE_LANDMARk_NAME))) {
          logWarning("Waiting landmark file ready %s", (b3di::FileUtils::appendPath(dir_arg, FACE_LANDMARk_NAME)).c_str());
          Sleep(30);
        }

        /* check if b3dCalibData.bin file exist first */
        while (!b3di::FileUtils::fileExists(b3di::FileUtils::appendPath(dir_arg, "CalibrationFiles/b3dCalibData.bin"))) {
          logWarning("Waiting b3dCalibData file ready %s", (b3di::FileUtils::appendPath(dir_arg, "CalibrationFiles/b3dCalibData.bin")).c_str());
          Sleep(30);
        }

        //initTracker(trackHeadInput, trackHeadOutput);     // initTracker after first depth gets computed

        initDepthProcessor(_depthProcessor);

        int currFileIndex = 0;
        for (;;) {
          stringstream ssFront;
          ssFront << setw(3) << setfill('0') << currFileIndex+1;          
          stringstream ssBack;
          ssBack << setw(5) << setfill('0') << currFileIndex+1;
          String nextFileNamePattern = "^"+ssFront.str()+"_([^_]+)_"+ssBack.str()+"\\.raw$";
          String nextFileName = "";
          logInfo("nextFileNamePattern: %s", nextFileNamePattern.c_str());
          if (lookForNextFile(currFileIndex, folder_to_monitor, nextFileNamePattern, nextFileName)) {
            if (currFileIndex < framecount_arg) {
              // process this file asynchronously (in a thread)
              processFileAsync(nextFileName, folder_to_monitor, folderNames);

              //// process this file in sequence
              //processFile(nextFileName, folder_to_monitor, folderNames);
            }
          }
          Sleep(30);
        }
        Sleep(500);
    }

    logWarning("Exit Main Thread");
    return 0;
}