#include "B3DCamera_B3D4.h"

#include <stdlib.h>     /* getenv */
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <b3ddepth/utils/TLog.h>

#include <b3dutils/log.h>
#include <b3dutils/common_utils.h>

#ifdef __ANDROID__
#include "sys/system_properties.h"
#include "backTrace.h"
#endif

using namespace std;
using namespace b3dd;
using namespace cv;

namespace b3d4 {

const std::string B3DCamera_B3D4::TAG = "DepthCamera_B3D4";

static std::string FILE_PATH_PREFIX;

bool B3DCamera_B3D4::detectDevice() {
    return true;
}


B3DCamera_B3D4::B3DCamera_B3D4(DCamType type) :
    _type(type)
    ,_currentDepthCompuTime(0.0)
    ,_resetDepthProfile(false)
{
#ifdef ANDROID
    _currentFrameTypes = { B3DCameraFrame::STEREO_FRAME, B3DCameraFrame::M_FRAME };
#else
    _currentFrameTypes = { B3DCameraFrame::STEREO_FRAME, B3DCameraFrame::M_FRAME };
#endif

    _deviceID   = "";
    _deviceName = "";
    _streamRGB  = _currentFrameTypes.find(B3DCameraFrame::M_FRAME) != _currentFrameTypes.end();
    _decodeType = DecodeType::SINGLEVIEW;

    if(!(b3dutils::getPackageName().compare("com.bellus3d.android.calibrationtool")))
        FILE_PATH_PREFIX = "/sdcard/Bellus3d/B3D4/calibrationtool/";
    else
        FILE_PATH_PREFIX = "/sdcard/B3D4/";

    setIsCancel(false);
#ifdef __ANDROID__
    signal(SIGSEGV, signalHandler);
#endif
}


B3DCamera_B3D4::~B3DCamera_B3D4() {
    dispose();
}


std::string B3DCamera_B3D4::getDepthStatsString() {

    std::string depthStats = "FPS: " + b3dutils::to_string(getCurrentFPS()) +
        ", per frame takes " + b3dutils::to_string(_currentDepthCompuTime) + " ms";
    return depthStats;
}

void B3DCamera_B3D4::addFrameToQueue(double currentFrameTime) {

    if (_previousFramesTimestamps.size() == TIMESTAMP_QUEUE_SIZE) {
        _previousFramesTimestamps.pop();
    }
    _previousFramesTimestamps.push(currentFrameTime);
}

B3DCameraError B3DCamera_B3D4::connectSync() {

    std::string methodName = "DepthCamera::connectSync()";
    return B3DCameraError();
}

void B3DCamera_B3D4::dispose() {

    string currentStateStr = getDepthCameraStateString(_currentCameraState);

    logInfo("called at state %s", currentStateStr.c_str());

}


B3DCameraError B3DCamera_B3D4::openSync() {

    std::string methodName = "DepthCamera::openSync()";
    return B3DCameraError();
}


B3DCameraError B3DCamera_B3D4::closeSync() {

    std::string methodName = "DepthCamera::closeSync()";
    return B3DCameraError();
}


B3DCameraError B3DCamera_B3D4::startStreamSync() {

    std::string methodName = "DepthCamera::startStreamSync()";
    return B3DCameraError();
}


B3DCameraError B3DCamera_B3D4::stopStreamSync() {

    std::string methodName = "DepthCamera::stopStreamSync()";
    return B3DCameraError();
}

std::string B3DCamera_B3D4::getDeviceID() {
    // Print warning
    if (_deviceID.empty()) {
        logWarning("fails - deviceId not available yet (empty string)");
    }

    return _deviceID;
}


std::string B3DCamera_B3D4::getDeviceName() {

    if (!_deviceName.empty()) { // deviceName has been updated
        logWarning("fails - device name not available yet, nullptr");
        return _deviceName;
    }

    return _deviceName;
}

std::string B3DCamera_B3D4::getFolderPath() {
    return FILE_PATH_PREFIX;
}

void B3DCamera_B3D4::loadDeviceInfo(const std::string& deviceId) {

}


void B3DCamera_B3D4::loadCalibrationFiles() {
}


void B3DCamera_B3D4::resetFrameTime() {
    logInfo("called %f", b3dutils::now_ms());
}

void B3DCamera_B3D4::printTiming(double totalTime, const string& operationMessage) {

    logInfo("%s takes %f msec", operationMessage.c_str(),totalTime);
}

void B3DCamera_B3D4::printPerFrameInfo(const std::string& perFrameMessage) {
    logInfo("%s", perFrameMessage.c_str());
}

B3DCameraError B3DCamera_B3D4::setProjectorLevel(ProjectorLevel level) {

    std::string methodName = "DepthCamera_B3D4::setProjectorLevel()";

    return B3DCameraError{};
}

void B3DCamera_B3D4::onFrame(B3DCameraFramePtr framePtr) {
    _streamListener->onFrame(framePtr);
}

void B3DCamera_B3D4::decodeFrames(int8_t* bytearray,int64_t frameCount, int64_t timeStamp, int32_t mResolution, uint8_t isCapture, int col,int row) {
    logVerbose("E");

    if(bytearray == nullptr) {
        logError("input image might be null");
        return;
    }

    if(getIsCancel()) {
        logWarning("cancel native processing !");
        return;
    }

    // byteData format:
    // 0 - bytesAvailable_M(COL_M*ROW_M*3/2) bytes -> YUV
    // follows 1024000 bytes of IR L
    // follows 1024000 bytes of IR R
    // 1920X1080,2592X1944,3264X2448
    int COL_M = 2448;
    int ROW_M = 3264;
    int bytesAvailable_M = COL_M*ROW_M*3/2;
    int COL_L = 800;
    int ROW_L = 1280;
    size_t bytesAvailable = COL_L*ROW_L;
    char value[255] = "";
#ifdef __ANDROID__
    __system_property_get("debug.bellus3d.calibrationtool.res",value);
    if(atoi(value) == 8 || mResolution == TripleCamResolution::RESOLUTION_8M) {
        COL_M = 2448;
        ROW_M = 3264;
        bytesAvailable_M = COL_M*ROW_M*3/2;
    } else if(atoi(value) == 2 || mResolution == TripleCamResolution::RESOLUTION_2M) {
        COL_M = 1224;
        ROW_M = 1632;
        bytesAvailable_M = COL_M*ROW_M*3/2;
    }
#endif

    FrameID frameIndex = static_cast<FrameID>(frameCount);
    FrameTime frameTimestamp = timeStamp;

    cv::Mat frameYUV = cv::Mat::zeros(ROW_M * 3 / 2, COL_M, CV_8UC1);
    cv::Mat frameRGB;
    cv::Mat frameL = cv::Mat::zeros(ROW_L, COL_L, CV_8UC1);
    cv::Mat frameR = cv::Mat::zeros(ROW_L, COL_L, CV_8UC1);

    long timeStampM = 0;
    long timeStampL = 0;
    long timeStampR = 0;

    if(_decodeType != DecodeType::DIAGNOSTIC) {
        /* singleview had different decode rule */
        if (_decodeType == DecodeType::SINGLEVIEW      ||
            _decodeType == DecodeType::FACEDETECTION   ||
            _decodeType == DecodeType::CALIBRATIONTOOL ||
            _decodeType == DecodeType::FACELANDMARK) {
            if (bytearray) {
                memcpy(frameYUV.data, (unsigned char *) bytearray,
                       bytesAvailable_M * sizeof(unsigned char));
                cv::cvtColor(frameYUV, frameRGB, CV_YUV2BGR_NV21);
            } else {
                logError("bytearray is null on M buffer, decodeType %d, frameIndex %d", _decodeType,
                         frameIndex);
                //TODO should handle this case further
            }
        }

#ifdef __ANDROID__
        __system_property_get("persist.vendor.bellus3d.capture.raw", value);
#endif

        if (isCapture && (atoi(value) == 1)) {
            FILE *pf1, *pf2, *pf3;
            unsigned char *rgb = static_cast<unsigned char *>(malloc(
                    bytesAvailable_M * sizeof(unsigned char *)));
            unsigned char *irL = static_cast<unsigned char *>(malloc(
                    bytesAvailable * sizeof(unsigned char *)));
            unsigned char *irR = static_cast<unsigned char *>(malloc(
                    bytesAvailable * sizeof(unsigned char *)));
            unsigned char *temp = (unsigned char *) bytearray;
            memcpy(rgb, temp, bytesAvailable_M * sizeof(unsigned char));
            temp += bytesAvailable_M;

            memcpy(irL, temp, bytesAvailable * sizeof(unsigned char));
            temp += bytesAvailable;

            memcpy(irR, temp, bytesAvailable * sizeof(unsigned char));
            temp += bytesAvailable;

            const std::string timeStamp = std::to_string(frameTimestamp) + "_";

            pf1 = fopen((FILE_PATH_PREFIX + "M/" + timeStamp + "_M.yuv").c_str(), "wb+");
            pf2 = fopen((FILE_PATH_PREFIX + "L/" + timeStamp + "_L.raw").c_str(), "wb+");
            pf3 = fopen((FILE_PATH_PREFIX + "R/" + timeStamp + "_R.raw").c_str(), "wb+");

            fwrite(rgb, bytesAvailable_M * sizeof(unsigned char), 1, pf1);
            fwrite(irL, bytesAvailable * sizeof(unsigned char), 1, pf2);
            fwrite(irR, bytesAvailable * sizeof(unsigned char), 1, pf3);
        }

        // Extract L frame
        if (_decodeType == DecodeType::SINGLEVIEW      ||
            _decodeType == DecodeType::FACEDETECTION   ||
            _decodeType == DecodeType::CALIBRATIONTOOL ||
            _decodeType == DecodeType::FACELANDMARK) {
            bytearray += bytesAvailable_M;
        }


        if (bytearray) {
            memcpy(frameL.data, bytearray, bytesAvailable * sizeof(unsigned char));
        } else {
            logError("bytearray is null on L IR buffer, decodeType %d, frameIndex %d", _decodeType,
                     frameIndex);
            //TODO should handle this case further
        }

        // Extract R frame
        bytearray += bytesAvailable;

        if (bytearray) {
            memcpy(frameR.data, bytearray, bytesAvailable * sizeof(unsigned char));
        } else {
            logError("bytearray is null on R IR buffer, decodeType %d, frameIndex %d", _decodeType,
                     frameIndex);
            //TODO should handle this case further
        }

        // move pointer to image buffer end, prepare parse timestamp
        bytearray += bytesAvailable;

        if (bytearray) {
            // timestampM
            memcpy(&timeStampM, bytearray, sizeof(long));
            bytearray += 8;

            // timestampL
            memcpy(&timeStampL, bytearray, sizeof(long));
            bytearray += 8;

            // timestampR
            memcpy(&timeStampR, bytearray, sizeof(long));
        } else
            logWarning("no timestamp info");

        logVerbose("tM : %ld, tL : %ld, tR : %ld", timeStampM, timeStampL, timeStampR);
    } else {
        if(diagnosticM.empty())
            diagnosticM = imread(DIAGNOSTIC_FILE_PATH + "diagnosticM.jpg");
        frameRGB = diagnosticM;

        if(diagnosticL.empty())
            diagnosticL = imread(DIAGNOSTIC_FILE_PATH + "diagnosticL.png", CV_LOAD_IMAGE_GRAYSCALE);
        frameL = diagnosticL;

        if(diagnosticR.empty())
            diagnosticR = imread(DIAGNOSTIC_FILE_PATH + "diagnosticR.png", CV_LOAD_IMAGE_GRAYSCALE);
        frameR = diagnosticR;
    }

    B3DCameraFramePtr LFramePtr(new B3DCameraFrame(B3DCameraFrame::FrameType::L_FRAME));
    LFramePtr->frameImage = frameL;
    LFramePtr->frameInfo.frameId   = frameIndex;
    LFramePtr->frameInfo.frameTime = static_cast<FrameTime>(timeStampL);
    LFramePtr->frameInfo.frameType = (bool)isCapture ? FrameInfo::FrameType::CAPTURE : FrameInfo::FrameType::PREVIEW;
    //LFramePtr->isValid = (bool)isValid;
    this->onFrame(LFramePtr);

    B3DCameraFramePtr RFramePtr(new B3DCameraFrame(B3DCameraFrame::FrameType::R_FRAME));
    RFramePtr->frameImage = frameR;
    RFramePtr->frameInfo.frameId   = frameIndex;
    RFramePtr->frameInfo.frameTime = static_cast<FrameTime>(timeStampR);
    RFramePtr->frameInfo.frameType = (bool)isCapture ? FrameInfo::FrameType::CAPTURE : FrameInfo::FrameType::PREVIEW;
    //RFramePtr->isValid = (bool)isValid;
    this->onFrame(RFramePtr);

    B3DCameraFramePtr MFramePtr(new B3DCameraFrame(B3DCameraFrame::FrameType::M_FRAME));
    MFramePtr->frameImage = frameRGB;
    MFramePtr->frameInfo.frameId   = frameIndex;
    MFramePtr->frameInfo.frameTime = static_cast<FrameTime>(timeStampM);
    MFramePtr->frameInfo.frameType = (bool)isCapture ? FrameInfo::FrameType::CAPTURE : FrameInfo::FrameType::PREVIEW;
    //MFramePtr->isValid = (bool)isValid;
    this->onFrame(MFramePtr);
    logVerbose("X");
}

void B3DCamera_B3D4::setDecodeType(DecodeType type) {
    logVerbose("E");
    _decodeType = type;
    logVerbose("set decode type %d",_decodeType);
    logVerbose("X");
}

}  // End of b3d namespace
