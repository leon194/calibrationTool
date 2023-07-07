#include "B3DCamera_Files.h"

#include <stdlib.h>     /* getenv */

#include <b3ddepth/utils/TLog.h>

#include <b3dutils/log.h>
#include <b3dutils/common_utils.h>
#ifdef __ANDROID__
#include "backTrace.h"
#endif

using namespace std;
using namespace b3dd;


namespace b3d4 {

const std::string B3DCamera_Files::TAG = "DepthCamera_Files";

static const std::string DCAM_CALIB_FILES[3]  = { "leftCam.yml", "rightCam.yml", "midCam.yml" };


bool B3DCamera_Files::detectDevice() {
    return true;
}


B3DCamera_Files::B3DCamera_Files(DCamType type) {
    logVerbose("E");
    _type = type;

#ifdef ANDROID
    _currentFrameTypes = { B3DCameraFrame::STEREO_FRAME, B3DCameraFrame::M_FRAME };
#else
    _currentFrameTypes = { B3DCameraFrame::STEREO_FRAME, B3DCameraFrame::M_FRAME };
    //_currentFrameTypes = { B3DCameraFrame::L_FRAME, B3DCameraFrame::R_FRAME, B3DCameraFrame::M_FRAME };
#endif

    _currentDepthCompuTime = 0.0;


    _deviceID      = "";
    _deviceName    = "";


    _streamRGB = _currentFrameTypes.find(B3DCameraFrame::M_FRAME) != _currentFrameTypes.end();
#ifdef __ANDROID__
    signal(SIGSEGV, signalHandler);
#endif
}


B3DCamera_Files::~B3DCamera_Files() {
    dispose();
}


std::string B3DCamera_Files::getDepthStatsString() {

    std::string depthStats = "FPS: " + b3dutils::to_string(getCurrentFPS()) +
        ", per frame takes " + b3dutils::to_string(_currentDepthCompuTime) + " ms";
    return depthStats;
}

void B3DCamera_Files::addFrameToQueue(double currentFrameTime) {

    if (_previousFramesTimestamps.size() == TIMESTAMP_QUEUE_SIZE) {
        _previousFramesTimestamps.pop();
    }
    _previousFramesTimestamps.push(currentFrameTime);
}

B3DCameraError B3DCamera_Files::connectSync() {

    std::string methodName = "DepthCamera::connectSync()";
    return B3DCameraError();
}

void B3DCamera_Files::dispose() {

    string currentStateStr = getDepthCameraStateString(_currentCameraState);

    logInfo("called at state %s", currentStateStr.c_str());

}


B3DCameraError B3DCamera_Files::openSync() {

    std::string methodName = "DepthCamera::openSync()";
    return B3DCameraError();
}


B3DCameraError B3DCamera_Files::closeSync() {

    std::string methodName = "DepthCamera::closeSync()";
    return B3DCameraError();
}


B3DCameraError B3DCamera_Files::startStreamSync() {

    std::string methodName = "DepthCamera::startStreamSync()";

#ifdef WIN32
    _datadir = "../../data/SampleDatasets/20190507_c/";
#else
    logInfo("%s", _datadir.c_str());
#endif

    framesL.initialize(_datadir + "camera/L/", "L_", "", b3di::ImageContainer::IMAGE_GRAY, false);
    framesR.initialize(_datadir + "camera/R/", "R_", "", b3di::ImageContainer::IMAGE_GRAY, false);
    framesRGB.initialize(_datadir + "camera/M/", "M_", "", b3di::ImageContainer::IMAGE_RGB, false);
    logVerbose("X");


    FrameTime frameTimestamp = 0.0L;
    bool isValid = true;
    for (int x = 0; x < framesL.getCount(); x++) {
        B3DCameraFramePtr LFramePtr(new B3DCameraFrame(B3DCameraFrame::FrameType::L_FRAME));
        LFramePtr->frameImage = framesL.getImage(x);
        LFramePtr->frameInfo.frameId = x;
        LFramePtr->frameInfo.frameTime = frameTimestamp;
        LFramePtr->isValid = (bool)isValid;
        this->onFrame(LFramePtr);


        B3DCameraFramePtr RFramePtr(new B3DCameraFrame(B3DCameraFrame::FrameType::R_FRAME));
        RFramePtr->frameImage = framesR.getImage(x);
        RFramePtr->frameInfo.frameId = x;
        RFramePtr->frameInfo.frameTime = frameTimestamp;
        RFramePtr->isValid = (bool)isValid;
        this->onFrame(RFramePtr);


        B3DCameraFramePtr MFramePtr(new B3DCameraFrame(B3DCameraFrame::FrameType::M_FRAME));
        MFramePtr->frameImage = framesRGB.getImage(x);
        MFramePtr->frameInfo.frameId = x;
        MFramePtr->frameInfo.frameTime = frameTimestamp;
        MFramePtr->isValid = (bool)isValid;
        this->onFrame(MFramePtr);
    }

    return B3DCameraError();
}


B3DCameraError B3DCamera_Files::stopStreamSync() {

    std::string methodName = "DepthCamera::stopStreamSync()";
    return B3DCameraError();
}
std::string B3DCamera_Files::getDeviceID() {

    if (_deviceID.empty()) {
        logWarning("fails - deviceId not available yet (empty string)");
    }

    return _deviceID;
}

std::string B3DCamera_Files::getDeviceName() {

    if (!_deviceName.empty()) { // deviceName has been updated
        logWarning("fails - device name not available yet, nullptr");
        return _deviceName;
    }

    return _deviceName;
}

std::string B3DCamera_Files::getFolderPath() {
    return _datadir;
}


void B3DCamera_Files::loadDeviceInfo(const std::string& deviceId) {

}


void B3DCamera_Files::loadCalibrationFiles() {
}


void B3DCamera_Files::resetFrameTime() {
    logInfo("called %f", b3dutils::now_ms());
}

void B3DCamera_Files::printTiming(double totalTime, const string& operationMessage) {

    logInfo("%s takes %f msec", operationMessage.c_str(),totalTime);
}

void B3DCamera_Files::printPerFrameInfo(const std::string& perFrameMessage) {
    logInfo("%s", perFrameMessage.c_str());
}

B3DCameraError B3DCamera_Files::setProjectorLevel(ProjectorLevel level) {

    std::string methodName = "DepthCamera_B3D4::setProjectorLevel()";

    return B3DCameraError{};
}

void B3DCamera_Files::onFrame(B3DCameraFramePtr framePtr) {
    _streamListener->onFrame(framePtr);
}

void B3DCamera_Files::decodeFrames(int8_t* bytearray,int64_t frameCount, int64_t timeStamp, int32_t mResolution, uint8_t isCapture, int col,int row) {
}

void B3DCamera_Files::setDecodeType(DecodeType type) {
}

}  // End of b3d namespace
