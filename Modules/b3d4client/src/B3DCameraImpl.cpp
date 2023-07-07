#include "B3DCameraImpl.h"

#include <algorithm>  // std::remove

// B3D Utils
#include <b3dutils/log.h>
#include <b3dutils/common_utils.h>


#include "B3DCameraError.h"


using namespace std;


namespace b3d4 {

B3DCameraImpl::B3DCameraImpl() :
    _currentCameraState(B3DCameraState::StateType::DISCONNECTED)
	,_deviceID("")
	,_deviceName("")
	,_isDepthProcessorSettingsUpdated(true)
	,_computeDepthTime(0.0){}


void B3DCameraImpl::registerStreamListener(B3DCameraStreamListenerPtr listener) {
    _streamListener = listener;
}


void B3DCameraImpl::unregisterStreamListener(B3DCameraStreamListenerPtr listener) {
    _streamListener = nullptr;
}


void B3DCameraImpl::registerObserver(B3DCameraObserverPtr observer) {
    // Can register multiple observers
    _observers.push_back(observer);
}


void B3DCameraImpl::unregisterObserver(B3DCameraObserverPtr observer) {
    // remove a particular observer from the list (test if the observer doesn't exist in list)
    _observers.erase(std::remove(_observers.begin(), _observers.end(), observer), _observers.end());
}


void B3DCameraImpl::setStreamTypes(const set<B3DCameraFrame::FrameType> frameTypes) {
    // TODO:
    // should check  DepthCamera state here, disallow the change during streaming
    _currentFrameTypes = frameTypes;
}

B3DCameraState::StateType B3DCameraImpl::getCurrentStateType() const {
    return _currentCameraState;
}

std::string B3DCameraImpl::getDepthStatsString() const {

    std::string depthStats =
        "IR FPS - " + b3dutils::to_string(getCurrentFPS(), /*precision*/ 2) +
        ", ComputeDepth - " + b3dutils::to_string(_computeDepthTime, /*precision*/ 2) + " ms";

    return depthStats;
}


//void DepthCameraImpl::setDepthMapSettings(DepthMapSettingsPtr settingsPtr) {
//
//    // Cast DepthMapSettingsPtr into DepthMapSettingsImplPtr
//    DepthMapSettingsImplPtr impl = static_pointer_cast<DepthMapSettingsImpl> (settingsPtr);
//
//    // Pass the object (pointed by settingsPtr) to HeadScannerSettings copy constructor
//    // So that a new HeadScannerSettings is created and wrapped in shared_ptr
//    // internal settingsPtr now points to this copy of input settings
//    _depthMapSettingsPtr = std::make_shared<DepthMapSettingsImpl>(*impl);
//
//    // Make necessary updates to "_depthConfigs"
//}

//
//DepthConfigExtPtr DepthCameraImpl::getDepthProcessorSettings() {
//
//    if (_depthConfigPtr == nullptr) {
//        TLog::log(TLog::LOG_WARNING, "DepthCameraImpl::getDepthProcessorSettings() - depthConfigPtr==nullptr!! using DEFAULT processing parameters!\n\n");
//        _depthConfigPtr = std::make_shared<DepthConfigExt>();
//    }
//
//    // Pass the object (pointed by _depthConfigPtr) to DepthConfigExt copy constructor
//    // So that a new DepthConfigExt is created and wrapped in shared_ptr
//    DepthConfigExtPtr depthConfigPtr = std::make_shared<DepthConfigExt>(*_depthConfigPtr);
//
//    // Return allocated copy of internal settings (wrapped in shared_ptr) back to application
//    return depthConfigPtr;
//}
//
//void DepthCameraImpl::setDepthProcessorSettings(DepthConfigExtPtr depthConfigPtr) {
//    
//    if (depthConfigPtr == nullptr) {
//        TLog::log(TLog::LOG_WARNING, "DepthCameraImpl::setDepthProcessorSettings() - nullptr!! no-op");
//        return;
//    }
//
//    // Pass the instance (pointed by depthConfigPtr) to DepthConfigExt copy constructor
//    // So that a new DepthConfigExt is created and wrapped in shared_ptr
//    // internal "_depthConfigPtr" now points to this copy of input settings
//    _depthConfigPtr = std::make_shared<DepthConfigExt>(*depthConfigPtr);
//
//    _isDepthProcessorSettingsUpdated = true;
//}



void B3DCameraImpl::cacheB3DCameraFrame(B3DCameraFramePtr framePtr) {


    if (_cachedFrames.size() == CACHE_FRAME_SIZE) {
        // Remove oldest frame, lock before removing
        try {
            std::lock_guard<std::mutex> lck(_cacheFramesMutex);
            _cachedFrames.pop();
        }
        catch (std::logic_error& err) {
            B3DLOGE << "cacheB3DCameraFrame() error: " << err.what();
            return;
        }
    }

    _cachedFrames.push(framePtr);
}



void B3DCameraImpl::addFrameTimestampToQueue(double currentFrameTime) {
    if (_frameTimestamps.size() == TIMESTAMP_QUEUE_SIZE) {
        _frameTimestamps.pop();
    }
    _frameTimestamps.push(currentFrameTime);
}

double B3DCameraImpl::getCurrentFPS() const {
    // Check how many frame's timestamp has been stored in queue
    // return immediately if there is less than 1 frame timestamp in the queue
    int frameIntervalCount = int(_frameTimestamps.size() - 1);
    if (frameIntervalCount <= 0) {
        return 0.0;
    }

    // Check time interval of the frame timestamps in the queue
    double timeInterval = _frameTimestamps.back() - _frameTimestamps.front();
    // calculate FPS and return
    return frameIntervalCount / (timeInterval / 1000.0);
}



void B3DCameraImpl::loadCalibrationData(const std::string& calibDataPath) {

//CameraCalibPtr cameraCalibPtr = shared_ptr<CameraCalib>(new CameraCalib());
//cameraCalibPtr->loadFromFile(calibDataPath);

//_depthProcessor.loadCalibrationData(cameraCalibPtr);
}


void B3DCameraImpl::updateDepthProcessorSettings() {

    // pass "_depthMapSettingsPtr" -> "_depthProcessorSettings"


    // EXT function
    // _depthProcessor.updateProcessingParams();

}




void B3DCameraImpl::setCurrentStateType(B3DCameraState::StateType currentStateType) {
    
    _currentCameraState = currentStateType;

    B3DLOGI << "DepthCameraImpl::setCurrentStateType: " << getDepthCameraStateString(currentStateType);
}

std::string B3DCameraImpl::getDepthCameraStateString(B3DCameraState::StateType stateType) {

    switch (stateType)
    {
    case B3DCameraState::StateType::DISCONNECTED:
        return "DISCONNECTED";
    case B3DCameraState::StateType::CONNECTING:
        return "CONNECTING";
    case B3DCameraState::StateType::DISCONNECTING:
        return "DISCONNECTING";
    case B3DCameraState::StateType::CONNECTED:
        return "CONNECTED";
    case B3DCameraState::StateType::OPENING:
        return "OPENING";
    case B3DCameraState::StateType::CLOSING:
        return "CLOSING";
    case B3DCameraState::StateType::OPEN:
        return "OPEN";
    case B3DCameraState::StateType::STARTING:
        return "STARTING";
    case B3DCameraState::StateType::STOPPING:
        return "STOPPING";
    case B3DCameraState::StateType::STREAMING:
        return "STREAMING";
    }
    return "";
}


set<B3DCameraFrame::FrameType> B3DCameraImpl::getStreamTypes() const {
    return _currentFrameTypes;
}


B3DCameraError B3DCameraImpl::checkCameraState(

    B3DCameraState::StateType acceptableState, const string& methodName) {

    if (_currentCameraState != acceptableState) {

        string currentStateStr  = getDepthCameraStateString(_currentCameraState);
        string expectedStateStr = getDepthCameraStateString(acceptableState);

        std::string errorMessage = methodName + " called at invalid state - current state: " + currentStateStr +
                                               ", acceptable state: " + expectedStateStr;

        return B3DCameraError(B3DCameraError::INVALID_STATE_ERROR, errorMessage);
    }
    else {
        return B3DCameraError();
    }
}

void B3DCameraImpl::checkDepthCameraState(
    B3DCameraState::StateType acceptableState, const std::string& methodName) {

    if (_currentCameraState != acceptableState) {

        string currentStateStr  = getDepthCameraStateString(_currentCameraState);
        string expectedStateStr = getDepthCameraStateString(acceptableState);

        std::string errorMessage = methodName + " called at invalid state - current state: " + currentStateStr +
                                  ", acceptable state: " + expectedStateStr;

        throw B3DCameraError(B3DCameraError::INVALID_STATE_ERROR, errorMessage);
    }
}


void B3DCameraImpl::updateDepthCameraState(B3DCameraState::StateType nextState, bool isReporting) {

    std::string methodName = "DepthCameraImpl::updateDepthCameraState()";

    std::string currentStateStr = getDepthCameraStateString(_currentCameraState);

    if (_currentCameraState == nextState) {
        B3DLOGI << "already in " + currentStateStr + " state. no-op.";
        return;
    }

    // Update current state
    _currentCameraState = nextState;
    currentStateStr = getDepthCameraStateString(_currentCameraState);

    B3DLOGI << currentStateStr;

    if (!isReporting) {
        return;
    }

    switch (_currentCameraState)
    {
    case B3DCameraState::StateType::DISCONNECTED:
        reportUpdateToAllObservers(DepthCameraDisconnectedState());
        break;
    case B3DCameraState::StateType::CONNECTING:
        reportUpdateToAllObservers(DepthCameraConnectingState());
        //statePtr = shared_ptr<DepthCameraConnectingState>(new DepthCameraConnectingState);
        break;
    case B3DCameraState::StateType::DISCONNECTING:
        reportUpdateToAllObservers(DepthCameraDisconnectingState());
        break;
    case B3DCameraState::StateType::CONNECTED:
        reportUpdateToAllObservers(DepthCameraConnectedState());
        break;
    case B3DCameraState::StateType::OPENING:
        reportUpdateToAllObservers(DepthCameraOpeningState());
        break;
    case B3DCameraState::StateType::CLOSING:
        reportUpdateToAllObservers(DepthCameraClosingState());
        break;
    case B3DCameraState::StateType::OPEN:
        reportUpdateToAllObservers(DepthCameraOpenState());
        break;
    case B3DCameraState::StateType::STARTING:
        reportUpdateToAllObservers(DepthCameraStartingState());
        break;
    case B3DCameraState::StateType::STOPPING:
        reportUpdateToAllObservers(DepthCameraStoppingState());
        break;
    case B3DCameraState::StateType::STREAMING:
        reportUpdateToAllObservers(DepthCameraStreamingState());
        break;
    default:
        break;
    }

    // reportUpdateToAllObservers(statePtr);
}


void B3DCameraImpl::reportUpdateToAllObservers(const B3DCameraState& depthCameraState) {

    // assert(statePtr);
    
    for (int i = 0; i < _observers.size(); i++)
        _observers[i]->onUpdate(depthCameraState, this);
}

void B3DCameraImpl::reportErrorToAllObservers(const B3DCameraError& b3dError) {

    for (int i = 0; i < _observers.size(); i++)
        _observers[i]->onError(b3dError, this);
}


bool B3DCameraImpl::isSetToStream(B3DCameraFrame::FrameType frameType) {

    // If can find "frameType" in set "_currentFrameTypes"
    if (_currentFrameTypes.find(frameType) != _currentFrameTypes.end()) {
        return true;
    }
    else {
        return false;
    }
}

//DepthCameraSettingsImplPtr DepthCameraImpl::getDepthCameraSettings() const {
//
//    // Pass the object (pointed by _settingsPtr) to DepthCameraSettings copy constructor
//    // So that a new DepthCameraSettings is created and wrapped in shared_ptr
//    DepthCameraSettingsImplPtr settingsPtr = 
//        std::make_shared<DepthCameraSettingsImpl>(*_settingsPtr);
//
//    // Return allocated copy of internal settings (wrapped in shared_ptr) back to application
//    return settingsPtr;
//}
//
//// Get depth camera sensor calibration data
//b3di::CameraParams DepthCameraImpl::getCameraParams(CameraType sensorType) const {
//    return _cameraParams[sensorType];
//}
//
//// Manually set current depth camera calibration data
//// DepthCamera will read default calibration data (on flash) when open()
//void DepthCameraImpl::setCameraParams(const b3di::CameraParams& params, CameraType sensorType) {
//    _cameraParams[sensorType] = params;
//}

//DepthCameraError DepthCameraImpl::restoreFactoryCalibrationData() {
//
//    std::string methodName = "DepthCameraImpl::restoreFactoryCalibrationData()";
//        cv::String reCalibFolder = FileUtils::appendPath
//        (getSDKPath(), RECALIB_DATA_DIR + CALIB_DATA_DIR + _deviceID);
//  
//        if (_deviceID.empty()) {
//            cv::String errorMessage = methodName + " error - device ID is not available yet.";
//            return DepthCameraError(DepthCameraError::INVALID_STATE_ERROR, errorMessage);
//        }
//
//        if (FileUtils::directoryExists(reCalibFolder)) {
//            for (int i = 0; i < 3; i++) {
//                cv::String reCalibFile = FileUtils::appendPath(reCalibFolder, CALIB_FILES[i]);
//                if (!FileUtils::deleteFile(reCalibFile)) {
//                    cv::String errorMessage = methodName + " error - file is missing <" + reCalibFile + ">";
//                    return DepthCameraError(DepthCameraError::FILE_IO_ERROR, errorMessage);
//                }
//            }
//            if (!FileUtils::deleteEmptyFolder(reCalibFolder)) {
//                cv::String errorMessage = methodName + " error - folder is not empty <" + reCalibFolder + ">";
//                return DepthCameraError(DepthCameraError::FILE_IO_ERROR, errorMessage);
//            }
//        }
//
//        return DepthCameraError();
//}


//void DepthCameraImpl::logMessage(TLog::LogType logType, std::string methodName, std::string debugMessage) const {
//    TLog::log(logType, "%s - %s", methodName.c_str(), debugMessage.c_str());
//}



B3DCameraError B3DCameraImpl::setExposureStereo(int microSeconds) {
    return B3DCameraError();
}

}  // End of namespace b3d
