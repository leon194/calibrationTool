#include "DepthCamera_Q845.h"

// #include <opencv2/opencv.hpp>

// B3D Util
#include "B3DFileIO.h"
#include "B3DUtils.h"
#include "TLog.h"
#include "TProfile.h"

using namespace std;

namespace b3dd {

const std::string DepthCamera_Q845::TAG = "DepthCamera_Q845";

const std::string CALIB_DATA_PATH_Q845 = "sdcard/B3DDepth/Seres/calib.data";

bool DepthCamera_Q845::detectDevice() {
    return false;
}

DepthCamera_Q845::DepthCamera_Q845() {

	// Do not set logLevel inside. It should be set by the app.
    TLog::setLogLevel(TLog::LOG_INFO);

    _deviceID   = "SERES0001";
    _deviceName = "SERES";

    // _settingsPtr->setDepthCameraType(DepthCameraSettings::DepthCameraType::SERES);

    _currentFrameTypes = { DepthCameraFrame::STEREO_FRAME, DepthCameraFrame::D_FRAME };


    // ---- Configure DepthProcessor
    loadCalibrationData(CALIB_DATA_PATH_Q845);


    TLog::log(TLog::LOG_ALWAYS, "DepthCamera_Q845 constructed in C++");
}

DepthCamera_Q845::~DepthCamera_Q845() {

}

void DepthCamera_Q845::connect() {

    std::string methodName = "DepthCamera::connect()";

}


void DepthCamera_Q845::disconnect() {

}


//void DepthCamera_SPRD::dispose() {
//
//}


void DepthCamera_Q845::open() {

    TProfile tp;
    tp.start();

    std::string methodName = "DepthCamera::open()";

}


void DepthCamera_Q845::close() {


    std::string methodName = "DepthCamera::close()";
}


DepthCameraError DepthCamera_Q845::startStreamSync() {

    std::string methodName = "DepthCamera_Q845::startStreamSync()";

    TLog::log(TLog::LOG_INFO, "%s called", methodName.c_str());


    _doStreamingThreadPtr = std::make_shared<std::thread>(DepthCamera_Q845::doStreaming, this);
    _doStreamingThreadPtr->detach();

    return DepthCameraError();
}


DepthCameraError DepthCamera_Q845::stopStreamSync() {

    std::string methodName = "DepthCamera::stopStreamSync()";

    return DepthCameraError();
}


//DepthCameraError DepthCamera_Q845::startStream() {
//    std::string methodName = "DepthCameraImpl::startStream()";
//    return DepthCameraError();
//}
//
//DepthCameraError DepthCamera_Q845::stopStream() {
//    std::string methodName = "DepthCamera::stopStream()";
//    return DepthCameraError();
//}


void DepthCamera_Q845::doStreaming(DepthCamera* depthCamera) {

    string methodName = "DepthCamera_Q845::doStreaming()";
    //double methodStartTime = now_ms();

    DepthCamera_Q845* impl = (DepthCamera_Q845*)depthCamera;


    // For FPS control
    const double UPDATE_FPS = 30.0;
    double updateInterval = 1000.0 / UPDATE_FPS;  // updates every 100ms
    double previousTime;

    // bool isComputeDepth = true;
    TProfile tpCompuDepth;

    //while (impl->getCurrentStateType() == DepthCameraState::StateType::STREAMING) {

    while(true) {

        // actual Loop starts
        previousTime = now_ms();  // FPS control clock starts

        // Only do things if stream listener is registered
        if (impl->_streamListener != nullptr) {

            DepthCameraFramePtr framePtr;

            try {
                // Check "_cachedFrames", continue loop if empty
                if (impl->_cachedFrames.empty()) {
                    continue;
                }
                // Lock "_cachedFrames" before retrieving the DepthFramePtr
                std::lock_guard<std::mutex> lck(impl->_cacheFramesMutex);
                framePtr = impl->_cachedFrames.front();
            }
            catch (std::logic_error& err) {
                TLog::log(TLog::LOG_ERROR, "doStreaming() error: %s", err.what());
                break;
            }


            // Check output stream settings: {L,R,M,D,STEREO}
            if (impl->isSetToStream(framePtr->frameType)) {
                // Stream current frame type
                impl->_streamListener->onFrame(framePtr);
            }

            // Check if we need to compute & stream depth frame
            if (framePtr->frameType == DepthCameraFrame::STEREO_FRAME) {
                // Add current frame timestamp to queue for FPS statistics
                impl->addFrameTimestampToQueue(framePtr->frameInfo.frameTime);

                if (impl->isSetToStream(DepthCameraFrame::D_FRAME)) {

                    // update DepthProcessor settings
                    if (impl->_isDepthProcessorSettingsUpdated) {
                        // pass new settings to depth processor
                        impl->_depthProcessor.updateProcessorSettingsExt(impl->_depthConfigPtr);
                        impl->_isDepthProcessorSettingsUpdated = false;
                    }

                    // Compute depth frame from IR stereo frames
                    double computeDepthStartTime = now_ms();

                    B3DImage depthImage;
                    impl->_depthProcessor.computeDepthExt(framePtr->frameImage, depthImage);

                    impl->_computeDepthTime = (now_ms() - computeDepthStartTime);


                    // Send computed Depth Frame to stream listener
                    DepthCameraFramePtr DFramePtr(new DepthCameraFrame(DepthCameraFrame::D_FRAME));
                    DFramePtr->frameImage = depthImage;
                    DFramePtr->frameInfo.frameId   = framePtr->frameInfo.frameId;
                    DFramePtr->frameInfo.frameTime = framePtr->frameInfo.frameTime;

                    impl->_streamListener->onFrame(DFramePtr);
                }
            }


        }  // End of if stream listener checking


        // FPS control
        double elapsedTime = now_ms() - previousTime;
        if (updateInterval > elapsedTime) {
            sleepFor((unsigned int)(updateInterval - elapsedTime));
        }

    }  // End of streaming while loop


}


void DepthCamera_Q845::onFrame(DepthCameraFramePtr framePtr) {

    // TODO: control Capture FPS here?

    if (_streamListener != nullptr) {

        // Check output stream settings: {L,R,M,D,STEREO}
        if (isSetToStream(framePtr->frameType)) {
            // Stream current frame type
            _streamListener->onFrame(framePtr);
        }

        // Check if we need to compute & stream depth frame
        if (framePtr->frameType == DepthCameraFrame::STEREO_FRAME) {
            // Add current frame timestamp to queue for FPS statistics
            addFrameTimestampToQueue(framePtr->frameInfo.frameTime);

            if (isSetToStream(DepthCameraFrame::D_FRAME)) {

                // update DepthProcessor settings
                if (_isDepthProcessorSettingsUpdated) {
                    // pass new settings to depth processor
                    _depthProcessor.updateProcessorSettingsExt(_depthConfigPtr);
                    _isDepthProcessorSettingsUpdated = false;
                }

                // Compute depth frame from IR stereo frames
                double computeDepthStartTime = now_ms();

                B3DImage depthImage;
                _depthProcessor.computeDepthExt(framePtr->frameImage, depthImage);

                _computeDepthTime = (now_ms() - computeDepthStartTime);


                // Send computed Depth Frame to stream listener
                DepthCameraFramePtr DFramePtr(new DepthCameraFrame(DepthCameraFrame::D_FRAME));
                DFramePtr->frameImage = depthImage;
                DFramePtr->frameInfo.frameId = framePtr->frameInfo.frameId;
                DFramePtr->frameInfo.frameTime = framePtr->frameInfo.frameTime;

                _streamListener->onFrame(DFramePtr);
            }
        }

        //printPerFrameInfo("DepthCamera_SPRD: sending frames to stream listener");
        // TLog::log(TLog::LOG_ERROR, "onFrame: %d", framePtr->frameType);


        //DepthCameraFramePtr LFramePtr(new DepthCameraFrame(DepthCameraFrame::L_FRAME));
        //LFramePtr->frameImage = frameL;
        //LFramePtr->frameInfo.frameId = current_frameId;
        //LFramePtr->frameInfo.frameTime = current_frameTime;

        //LFramePtr->isValid = _validStereoFrame;

        //_streamListener->onFrame(LFramePtr);

        //DepthCameraFramePtr RFramePtr(new DepthCameraFrame(DepthCameraFrame::R_FRAME));
        //RFramePtr->frameImage = frameR;
        //RFramePtr->frameInfo.frameId = current_frameId;
        //RFramePtr->frameInfo.frameTime = current_frameTime;

        //RFramePtr->isValid = _validStereoFrame;

        // _streamListener->onFrame(RFramePtr);
    }
}




}  // End of b3d namespace
