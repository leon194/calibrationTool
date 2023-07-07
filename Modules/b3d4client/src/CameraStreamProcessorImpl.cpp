#include "CameraStreamProcessorImpl.h"

/* b3di */
#include <b3di/B3d_API.h>
#include <b3di/FileUtils.h>
#include <b3di/depthtoobj.h>
#include <b3di/FrameStore.h>
#include <b3di/FaceTracker.h>
#include <b3di/FaceTrackerASM.h>
#include <b3di/DepthCamProps.h>
#include <b3di/SkinDetector.h>
#include <b3di/B3d_utils.h>
#include <b3di/B3d_files.h>

#include <b3dutils/log.h>


//* b3ddepth/utils */
#include <b3ddepth/utils/B3DFileIO.h>
#include <b3ddepth/utils/imageutils.h>
#include <b3ddepth/utils/TLog.h>

//* b3ddepth/core */
#include <b3ddepth/core/DepthProcessError.h>

/* b3d4client includes */
#include <b3d4client/B3DConfig.h>
#include <b3d4client/B3D4AutoCalib.h>

/* opencv includes */
#include <opencv2/core/utils/filesystem.hpp>

#ifdef __ANDROID__
#include "sys/system_properties.h"
#include <err.h>
#include <b3d4client/B3D4ClientJNIError.h>
#include "backTrace.h"
#ifdef ENCODEPNG
#include <opencv2/imgcodecs/imgcodecs_c.h>
#endif
#endif

using namespace std;

namespace b3d4 {

// Convert B3DImage to opencv Mat (make a copy)
void B3DImageToCvMat(const B3DImage& inImage, cv::Mat& outMat, const double imageScale = 1.0) {
    logVerbose("E");
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
            logError("Unsupported B3DImageType: %s ! ",type);
            return;
    }

    cv::Mat tmpMat = cv::Mat(inImage.rows(), inImage.cols(), matType);
    std::memcpy(tmpMat.data, inImage.data(), inImage.rowSize() * inImage.rows() * sizeof(unsigned char));
    const auto interType = type == B3DIMAGE_MONO ? cv::INTER_AREA : cv::INTER_NEAREST;
    cv::resize(tmpMat, outMat, cv::Size(), imageScale, imageScale, interType);
    logVerbose("X");
}

void CvMatToB3DImage(const cv::Mat& inMat, B3DImage& outImage, const double imageScale = 1.0) {
    logVerbose("E");
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
            logError("Unsupported B3DImageType: %s ! ",type);
            break;
    }

    // Construct B3DImage
    cv::Mat tmpMat;
    const auto interType = outType == B3DIMAGE_MONO ? cv::INTER_AREA : cv::INTER_NEAREST;
    cv::resize(inMat, tmpMat, cv::Size(), imageScale, imageScale, interType);
    outImage = B3DImage(tmpMat.rows, tmpMat.cols, outType);
    std::memcpy(outImage.data(), tmpMat.data, tmpMat.cols * tmpMat.rows * sizeof(unsigned char));
    logVerbose("X");
}


void saveIRL (std::string folderName, std::string timeStamp, int frameid, cv::Mat IRL) {
    logVerbose("E");
    std::string IRLName = "/Results/L/L_" + std::to_string(frameid) + "_" + timeStamp + ".png";
    cv::imwrite(folderName + IRLName, IRL);
    logVerbose("X");
}

void saveIRR (std::string folderName, std::string timeStamp, int frameid, cv::Mat IRR) {
    logVerbose("E");
    std::string IRLName = "/Results/R/R_" + std::to_string(frameid) + "_" + timeStamp + ".png";
    cv::imwrite(folderName + IRLName, IRR);
    logVerbose("X");
}

void saveDepth (std::string folderName, int frameid, cv::Mat depth) {
    logVerbose("E");
    std::string depthName = "/Results/D/D_" + std::to_string(frameid) + "_" + ".png";
    cv::imwrite(folderName + depthName, depth);
    logVerbose("X");
}

#define CHECKCANCEL(flag) if (!flag) {                  \
    logInfo("finishing %s",__func__);                   \
    if(_processListener)                                \
        _processListener->onProcessFinished(__func__);  \
    else logWarning("_processListener is null");        \
    return;                                             \
}

CameraStreamProcessorImpl::CameraStreamProcessorImpl() :
     faceRect(cv::Rect(-1,-1,-1,-1)){
    logVerbose("E");
    _stereoFramesCounter = 0;
    _max_process_frame_num = MAX_PROCESS_FRAME;
    _max_color_frame_num = MAX_COLOR_FRAME;
    _doDepth = false;
    _doFaceDetect = false;
    _doKeyFrameSelect = false;
    _doReCalibration = false;
    _doGenFaceLandMark = false;
    _doStreamDepth = false;
    _isProcessing = false;
    _isFaceDetectProcessing = false;
    _isGenFaceLandMarkProcessing = false;
    _isRecalibrationProcessing = false;
    _isStreamDepthProcessing = false;
    _reCalibrationReady = true;
    _needKeyFrameSelect = false;
    _needManualRecalib = false;
    _faceDetectReady = false;
    frameEnoughReported = false;
    _debugFlag = false;
#ifdef __ANDROID__
    signal(SIGSEGV, signalHandler);
#endif
    logVerbose("X");
}

CameraStreamProcessorImpl::~CameraStreamProcessorImpl() {
    logVerbose("E");
    logVerbose("X");
}

void CameraStreamProcessorImpl::initProcessParameters(const std::string rootdir, const std::string sessiondir,
        int processFrame, bool doKeyFrameSelection, bool needManualRecalib) {
    logVerbose("E");
    char value[255] = "";
    int debugSaveColor = 0;

    // load calib data
    _rootdir = rootdir;
    _calibrationdir = rootdir + "CalibrationFiles/";
    _sessiondir = sessiondir;
    _facDataDir = "/mnt/vendor/CalibrationFiles/";
    _needKeyFrameSelect = doKeyFrameSelection;
    _needManualRecalib = needManualRecalib;

    const std::string calibDataPath = _calibrationdir + "calib.data";


#ifdef __ANDROID__
    __system_property_get("debug.bellus3d.b3d4client.savecolor",value);
    debugSaveColor = atoi(value);
#endif
    if(debugSaveColor == 1)
        _max_color_frame_num = _max_process_frame_num = processFrame;
    else
        _max_process_frame_num = processFrame;

    _max_color_frame_num = (MAX_COLOR_FRAME > processFrame && processFrame > 0) ? processFrame : MAX_COLOR_FRAME;

    /* load calibration data */
    if(!_cameraCalibExtPtr)
        _cameraCalibExtPtr = std::shared_ptr<CameraCalibExt>(new CameraCalibExt());

    bool CalibrationValid = _cameraCalibExtPtr->loadFromFile(calibDataPath);

    if (!CalibrationValid || _cameraCalibExtPtr->getCalibDataVersion() != CALIB_DATA_VERSION)
    {
        if (!CalibrationValid)
        {
            logWarning("Fail to load calib data file %s",(calibDataPath).c_str());
        }
        else if (_cameraCalibExtPtr->getCalibDataVersion() != CALIB_DATA_VERSION)
        {
            logWarning("Calib data version [%.1f] -- Camera Calib version [%.1f]", \
            _cameraCalibExtPtr->getCalibDataVersion(), CALIB_DATA_VERSION);
        }

        logVerbose("Create calib data file from calib bin file");

        B3DCalibrationData calibBin;

        if (!readB3DCalibrationData(_facDataDir, CALIB_BIN_FILE_NAME, calibBin))
        {
            B3DNativeProcessError nativehErr =
                    B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_CALIBATION_DATA_INVALID,
                                          "Fail to load calib bin file " + _facDataDir + CALIB_BIN_FILE_NAME);
            logError("Fail to load calib bin file %s", (_facDataDir + CALIB_BIN_FILE_NAME).c_str());
            if(_processListener)
                _processListener->onError(nativehErr);
            return;
        }

        const B3DCalibrationData* calibBinPtr = &calibBin;

        _cameraCalibExtPtr->cacheCalibrationData(
                (unsigned char*)calibBinPtr,
                calibDataPath,
                DepthCameraType::DEPTHCAM_B3D4
        );

        if (!_cameraCalibExtPtr->loadFromFile(calibDataPath))
        {
            B3DNativeProcessError nativehErr =
                    B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_CALIBATION_DATA_INVALID,
                                          "Fail to load calib bin file " + _facDataDir + CALIB_BIN_FILE_NAME);
            logError("Fail to load calib data file %s", (calibDataPath).c_str());
            if(_processListener)
                _processListener->onError(nativehErr);
            return;
        }
    }

    _depthProcessor.loadCalibrationData(_cameraCalibExtPtr);

    /* config depth configs */
    _depthConfigPtr = std::shared_ptr<DepthConfigExt>(new DepthConfigExt());
    _depthConfigPtr->optimizeMode = OPTIMIZE_FULL_RES;
    _depthConfigPtr->depthRegistration = REGISTER_L;
    _depthConfigPtr->sceneMode = NORMAL;
    _depthConfigPtr->depthScale = 1.0f;
    _depthConfigPtr->depthUnit = 0.02f;
//    _depthConfigPtr->ROI_L = ROI(150, 160, 550, 700);

    // set internal settings
    _depthConfigPtr->useTwoDispMaps     = true;
    _depthConfigPtr->useDepthRange      = true;
    _depthConfigPtr->reduceDispBias     = true;
    _depthConfigPtr->useIRThres         = false;
    _depthConfigPtr->useForegroundMask  = true;

    _depthProcessor.updateProcessorSettingsExt(_depthConfigPtr);

    /* init face track */
    b3di::FaceTracker::FaceTrackerType landmarkType = b3di::FaceTracker::FT_ASM;
    logInfo("Detect face landmarks with tracker: FT_ASM");

    _ft = b3di::FaceTracker::newFaceTrackerPtr(landmarkType);

    if (!b3di::FaceTrackerASM::initTracker(_rootdir + STASM_FILE_PATH)) {
        B3DNativeProcessError nativehErr =
                B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_CONFIG_INVALID,
                                      "Invalid stasm config");
        logError("InitProcessParameters with error %s",(nativehErr.debugMessage).c_str());
        if(_processListener)
            _processListener->onError(nativehErr);
        return;
    }
    logVerbose("X");
}

void CameraStreamProcessorImpl::updateProcessorSettings(DepthConfigExtPtr depthConfigPtr) {
    logVerbose("E");
    _depthConfigPtr->updateDepthConfig(depthConfigPtr->depthCameraType);
    _depthProcessor.updateProcessorSettingsExt(depthConfigPtr);
    logVerbose("X");
};

void CameraStreamProcessorImpl::addFrame(const B3DCameraFramePtr framePtr) {
    logVerbose("E");
    logVerbose("_stereoFramesCounter %d _colorFrameStore %d", _stereoFramesCounter, _colorFrameStore.size());

    try {
        if (framePtr == nullptr) {
            logWarning("frame ptr is null");
            return;
        }
        // we don't need any more frames if we get enough frames
        if (_stereoFramesCounter >= _max_process_frame_num * 2 + 1 &&
            _colorFrameStore.size() >= _max_process_frame_num) {
            logVerbose("process frame enough _stereoFramesCounter %d _colorFrameStore %d",
                       _stereoFramesCounter, _colorFrameStore.size());
            return;
        }

        int frameid = (int) framePtr->frameInfo.frameId;

        switch (framePtr->frameType) {
            case B3DCameraFrame::FrameType::L_FRAME:
                if (_stereoComposer.find(frameid) == _stereoComposer.end()) {
                    cv::Mat dummy;
                    _stereoComposer[frameid] = std::make_pair(framePtr->frameImage.clone(), dummy);
                } else {
                    _stereoComposer[frameid].first = framePtr->frameImage.clone();
                }
                _stereoComposerStatus[frameid]++;
                ++_stereoFramesCounter;
                _timeStampL[frameid] = framePtr->frameInfo.frameTime;
                break;
            case B3DCameraFrame::FrameType::R_FRAME:
                if (_stereoComposer.find(frameid) == _stereoComposer.end()) {
                    cv::Mat dummy;
                    _stereoComposer[frameid] = std::make_pair(dummy, framePtr->frameImage.clone());
                } else {
                    _stereoComposer[frameid].second = framePtr->frameImage.clone();
                }

                _stereoComposerStatus[frameid]++;
                ++_stereoFramesCounter;
                _timeStampR[frameid] = framePtr->frameInfo.frameTime;
                break;
            case B3DCameraFrame::FrameType::M_FRAME:
                if (_colorFrameStore.size() < _max_color_frame_num /* hardcode first */) {
                    _colorFrameStore.push_back(framePtr->frameImage.clone());
                    _timeStampM[frameid] = framePtr->frameInfo.frameTime;
                }
                break;
            default:
                break;
        }

        /* if this frame id had receive 2 frames (L/R) we push frames into queue to process depth process */
        if (_stereoComposerStatus[frameid] == 2 &&
            (framePtr->frameType != B3DCameraFrame::FrameType::M_FRAME)) {
            B3DImage imagesL, imagesR;
            double imageScale = 0.75;

            logInfo("Frame ID: %d - scale %.2f", frameid, imageScale);

            if (_stereoComposer[frameid].first.size().width > 0 &&
                _stereoComposer[frameid].first.size().height > 0 &&
                _stereoComposer[frameid].second.size().width > 0 &&
                _stereoComposer[frameid].second.size().height > 0
                ) {
                CvMatToB3DImage(_stereoComposer[frameid].first, imagesL, imageScale);
                CvMatToB3DImage(_stereoComposer[frameid].second, imagesR, imageScale);
                if (_debugFlag) {
                    saveIRL(_sessiondir,
                            std::to_string((!_timeStampL.empty() && _timeStampL[frameid] > 0) ?
                                           _timeStampL[frameid] : 00000), frameid,
                            _stereoComposer[frameid].first);
                    saveIRR(_sessiondir,
                            std::to_string((!_timeStampR.empty() && _timeStampR[frameid] > 0) ?
                                           _timeStampR[frameid] : 00000), frameid,
                            _stereoComposer[frameid].second);
                }
                _stereoFrameStore.push(std::make_pair(imagesL, imagesR));
                _stereoFrameScale.push(imageScale);
            } else {
                logWarning("source image size might be zero !!");
            }
        }

        /* report that native side had coleted enough frame to process
         *  but still keep pass colorFrame to do in memory further
         *  if no need to do key frame select, just use the first color frame
         */
        if (_stereoFramesCounter == _max_process_frame_num * 2 &&
            !frameEnoughReported &&
            !_needKeyFrameSelect &&
            _colorFrameStore.size() > 0) {
            frameEnoughReported = true;
            if(_processListener)
                _processListener->onFrameEnough(_colorFrameStore, 0, 0);
            else
                logWarning("_processListener is null");
        }
    } catch( cv::Exception& e ) {
        B3DNativeProcessError nativehErr;
        const char* err_msg = e.what();
        nativehErr = B3DNativeProcessError(
                B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                err_msg);
        logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
        if(_processListener)
            _processListener->onError(nativehErr);
        return;
    } catch (...) {
        auto expPtr = std::current_exception();
        B3DNativeProcessError nativehErr;
        try {
            if(expPtr) std::rethrow_exception(expPtr);
        } catch(const std::exception& e) {
            const char* err_msg = e.what();
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                    err_msg);
            logError("some error occur %s",(nativehErr.debugMessage).c_str());
            if(_processListener)
                _processListener->onError(nativehErr);
            return;
        }
    }

    logVerbose("X");
};

void CameraStreamProcessorImpl::addFrameFaceDetect(const B3DCameraFramePtr framePtr) {
    logVerbose("E");

    try {
        if (framePtr == nullptr) {
            logWarning("frame ptr is null");
            return;
        }

        int frameid = (int) framePtr->frameInfo.frameId;

        switch (framePtr->frameType) {
            case B3DCameraFrame::FrameType::L_FRAME:
                if (_stereoComposer.find(frameid) == _stereoComposer.end()) {
                    cv::Mat dummy;
                    _stereoComposer[frameid] = std::make_pair(framePtr->frameImage.clone(), dummy);
                    ++_stereoFramesCounter;
                } else {
                    _stereoComposer[frameid].first = framePtr->frameImage.clone();
                }
                _stereoComposerStatus[frameid]++;
                break;
            case B3DCameraFrame::FrameType::R_FRAME:
                if (_stereoComposer.find(frameid) == _stereoComposer.end()) {
                    cv::Mat dummy;
                    _stereoComposer[frameid] = std::make_pair(dummy, framePtr->frameImage.clone());
                    ++_stereoFramesCounter;
                } else {
                    _stereoComposer[frameid].second = framePtr->frameImage.clone();
                }
                _stereoComposerStatus[frameid]++;
                break;
            case B3DCameraFrame::FrameType::M_FRAME:
                _colorImages[frameid] = framePtr->frameImage.clone();
                _stereoComposerStatus[frameid]++;
                break;
            default:
                break;
        }

        /* if this frame id had receive 3frames (L M R) we push frames into queue to process depth process */
        if (_stereoComposerStatus[frameid] == 3 && !_faceDetectReady) {
            std::lock_guard<std::mutex> lock(_gAddFrameLock);
            double imageScale = 0.25f;
            B3DImage imagesL, imagesR;
            if (_stereoComposer[frameid].first.size().width > 0 &&
                _stereoComposer[frameid].first.size().height > 0 &&
                _stereoComposer[frameid].second.size().width > 0 &&
                _stereoComposer[frameid].second.size().height > 0
                ) {
                CvMatToB3DImage(_stereoComposer[frameid].first, imagesL, imageScale);
                CvMatToB3DImage(_stereoComposer[frameid].second, imagesR, imageScale);
                _stereoFrameStore.push(std::make_pair(imagesL, imagesR));
                _stereoFrameScale.push(imageScale);
                _colorImageStore.push(_colorImages[frameid].clone());
                _faceDetectReady = true;
            } else {
                logWarning("source image size might be zero !!");
            }

            // clear memory
            _stereoComposer.clear();
            _colorImages.clear();
            _stereoComposerStatus.clear();

            logVerbose("_stereoComposer %d _colorImages %d _stereoComposerStatus %d",
                       _stereoComposer.size(), _colorImages.size(), _stereoComposerStatus.size());
        }
    }  catch( cv::Exception& e ) {
        B3DNativeProcessError nativehErr;
        const char* err_msg = e.what();
        nativehErr = B3DNativeProcessError(
                B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                err_msg);
        logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
        if(_processListener)
            _processListener->onError(nativehErr);
        return;
    } catch (...) {
        auto expPtr = std::current_exception();
        B3DNativeProcessError nativehErr;
        try {
            if(expPtr) std::rethrow_exception(expPtr);
        } catch(const std::exception& e) {
            const char* err_msg = e.what();
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                    err_msg);
            logError("some error occur %s",(nativehErr.debugMessage).c_str());
            if(_processListener)
                _processListener->onError(nativehErr);
            return;
        }
    }

    logVerbose("X");
};

/* find land mark only need color image */
void CameraStreamProcessorImpl::addFrameGenFaceLandMark(const B3DCameraFramePtr framePtr) {
    logVerbose("E");

    if (framePtr == nullptr) {
        logWarning("frame ptr is null");
        return;
    }

    int frameid = (int)framePtr->frameInfo.frameId;

    switch(framePtr->frameType) {
        case B3DCameraFrame::FrameType::L_FRAME:
        case B3DCameraFrame::FrameType::R_FRAME:
            break;
        case B3DCameraFrame::FrameType::M_FRAME:
            if(frameid == 1) {
                _colorImageStore.push(framePtr->frameImage.clone());
            }
            break;
        default:
            break;
    }
    logVerbose("X");
}

void CameraStreamProcessorImpl::addFrameRecalibration(const B3DCameraFramePtr framePtr) {
    logVerbose("E");

    try {
        if (framePtr == nullptr) {
            logWarning("frame ptr is null");
            return;
        }

        int frameid = (int) framePtr->frameInfo.frameId;

        switch (framePtr->frameType) {
            case B3DCameraFrame::FrameType::L_FRAME:
                if (_stereoComposer.find(frameid) == _stereoComposer.end()) {
                    cv::Mat dummy;
                    _stereoComposer[frameid] = std::make_pair(framePtr->frameImage.clone(), dummy);
                    ++_stereoFramesCounter;
                } else {
                    _stereoComposer[frameid].first = framePtr->frameImage.clone();
                }
                _stereoComposerStatus[frameid]++;
                break;
            case B3DCameraFrame::FrameType::R_FRAME:
                if (_stereoComposer.find(frameid) == _stereoComposer.end()) {
                    cv::Mat dummy;
                    _stereoComposer[frameid] = std::make_pair(dummy, framePtr->frameImage.clone());
                    ++_stereoFramesCounter;
                } else {
                    _stereoComposer[frameid].second = framePtr->frameImage.clone();
                }
                _stereoComposerStatus[frameid]++;
                break;
            case B3DCameraFrame::FrameType::M_FRAME:
                _colorImages[frameid] = framePtr->frameImage.clone();
                _stereoComposerStatus[frameid]++;
                break;
            default:
                break;
        }

        if (_stereoComposerStatus[frameid] == 2 &&
            (framePtr->frameType != B3DCameraFrame::FrameType::M_FRAME)) {
            B3DImage imagesL, imagesR;
            if (_stereoComposer[frameid].first.size().width > 0 &&
                _stereoComposer[frameid].first.size().height > 0 &&
                _stereoComposer[frameid].second.size().width > 0 &&
                _stereoComposer[frameid].second.size().height > 0
                ) {
                CvMatToB3DImage(_stereoComposer[frameid].first, imagesL);
                CvMatToB3DImage(_stereoComposer[frameid].second, imagesR);
                _stereoFrameStore.push(std::make_pair(imagesL, imagesR));
            } else {
                logWarning("source image size might be zero !!");
            }
        }
    }  catch( cv::Exception& e ) {
        B3DNativeProcessError nativehErr;
        const char* err_msg = e.what();
        nativehErr = B3DNativeProcessError(
                B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                err_msg);
        logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
        if(_processListener)
            _processListener->onError(nativehErr);
        return;
    } catch (...) {
        auto expPtr = std::current_exception();
        B3DNativeProcessError nativehErr;
        try {
            if(expPtr) std::rethrow_exception(expPtr);
        } catch(const std::exception& e) {
            const char* err_msg = e.what();
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                    err_msg);
            logError("some error occur %s",(nativehErr.debugMessage).c_str());
            if(_processListener)
                _processListener->onError(nativehErr);
            return;
        }
    }

    logVerbose("X");
}

void CameraStreamProcessorImpl::addFrameStreamDepth(const B3DCameraFramePtr framePtr) {
    logVerbose("E");
    logVerbose("_stereoFramesCounter %d _colorFrameStore %d", _stereoFramesCounter, _colorFrameStore.size());

    try {
        if (framePtr == nullptr) {
            logWarning("frame ptr is null");
            return;
        }
        // we don't need any more frames if we get enough frames
        if (_stereoFramesCounter >= _max_process_frame_num * 2 + 1 &&
            _colorFrameStore.size() >= _max_process_frame_num) {
            logVerbose("process frame enough _stereoFramesCounter %d _colorFrameStore %d",
                       _stereoFramesCounter, _colorFrameStore.size());
            return;
        }

        int frameid = (int) framePtr->frameInfo.frameId;

        switch (framePtr->frameType) {
            case B3DCameraFrame::FrameType::L_FRAME:
                if (_stereoComposer.find(frameid) == _stereoComposer.end()) {
                    cv::Mat dummy;
                    _stereoComposer[frameid] = std::make_pair(framePtr->frameImage.clone(), dummy);
                } else {
                    _stereoComposer[frameid].first = framePtr->frameImage.clone();
                }
                _stereoComposerStatus[frameid]++;
                ++_stereoFramesCounter;
                _stereoLFrameTimeStamp.push(framePtr->frameInfo.frameTime);
                _timeStampL[frameid] = framePtr->frameInfo.frameTime;
                break;
            case B3DCameraFrame::FrameType::R_FRAME:
                if (_stereoComposer.find(frameid) == _stereoComposer.end()) {
                    cv::Mat dummy;
                    _stereoComposer[frameid] = std::make_pair(dummy, framePtr->frameImage.clone());
                } else {
                    _stereoComposer[frameid].second = framePtr->frameImage.clone();
                }

                _stereoComposerStatus[frameid]++;
                ++_stereoFramesCounter;
                _timeStampR[frameid] = framePtr->frameInfo.frameTime;
                break;
            case B3DCameraFrame::FrameType::M_FRAME:
                break;
            default:
                break;
        }

        /* if this frame id had receive 2 frames (L/R) we push frames into queue to process depth process */
        if (_stereoComposerStatus[frameid] == 2 &&
            (framePtr->frameType != B3DCameraFrame::FrameType::M_FRAME)) {
            B3DImage imagesL, imagesR;
            double imageScale = 0.75f;
            logInfo("Frame ID: %d - scale %.2f", frameid, imageScale);
            if (_stereoComposer[frameid].first.size().width > 0 &&
                _stereoComposer[frameid].first.size().height > 0 &&
                _stereoComposer[frameid].second.size().width > 0 &&
                _stereoComposer[frameid].second.size().height > 0
                ) {
                CvMatToB3DImage(_stereoComposer[frameid].first, imagesL, imageScale);
                CvMatToB3DImage(_stereoComposer[frameid].second, imagesR, imageScale);
                if (_debugFlag) {
                    saveIRL(_sessiondir,
                            std::to_string((!_timeStampL.empty() && _timeStampL[frameid] > 0) ?
                                           _timeStampL[frameid] : 00000), frameid,
                            _stereoComposer[frameid].first);
                    saveIRR(_sessiondir,
                            std::to_string((!_timeStampR.empty() && _timeStampR[frameid] > 0) ?
                                           _timeStampR[frameid] : 00000), frameid,
                            _stereoComposer[frameid].second);
                }
                _stereoFrameStore.push(std::make_pair(imagesL, imagesR));
                _stereoFrameScale.push(imageScale);
            } else {
                logWarning("source image size might be zero !!");
            }
        }
    }  catch( cv::Exception& e ) {
        B3DNativeProcessError nativehErr;
        const char* err_msg = e.what();
        nativehErr = B3DNativeProcessError(
                B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                err_msg);
        logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
        if(_processListener)
            _processListener->onError(nativehErr);
        return;
    } catch (...) {
        auto expPtr = std::current_exception();
        B3DNativeProcessError nativehErr;
        try {
            if(expPtr) std::rethrow_exception(expPtr);
        } catch(const std::exception& e) {
            const char* err_msg = e.what();
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                    err_msg);
            logError("some error occur %s",(nativehErr.debugMessage).c_str());
            if(_processListener)
                _processListener->onError(nativehErr);
            return;
        }
    }
    logVerbose("X");
};

void CameraStreamProcessorImpl::InitProcessingParameter() {
    logVerbose("E");
    /* make sure previous process is clear, since we only init it at constructor */
    resetPrivateParameters();
    logVerbose("X");
};

void CameraStreamProcessorImpl::startProcessing() {
    logVerbose("E");
#if 0
    if (!_doReCalibration && _needManualRecalib) {
        _doReCalibration = true;
        _reCalibrationReady = false;
        _doReCalibrationThr = std::thread(&CameraStreamProcessorImpl::doReCalibration, this);
        _doReCalibrationThr.detach();
    }
#endif
    if (!_doKeyFrameSelect && _needKeyFrameSelect ) {
        _doKeyFrameSelect = true;
        _doKeyFrameSelectThr = std::thread(&CameraStreamProcessorImpl::doKeyFrameSelection, this, _max_color_frame_num);
        _doKeyFrameSelectThr.detach();
    }
	
    if (!_doDepth) {
        _doDepth = true;
        _timeStampM.resize(_max_process_frame_num);
        _timeStampL.resize(_max_process_frame_num);
        _timeStampR.resize(_max_process_frame_num);
        _doComputeThr = std::thread(&CameraStreamProcessorImpl::doDepthWork, this);
        _doComputeThr.detach();
    }

    _isProcessing = true;

    logVerbose("X");
};

void CameraStreamProcessorImpl::startFaceDetect() {
    logVerbose("E");
    if (!_doFaceDetect) {
        _doFaceDetect = true;
        _doFaceDetectThr = std::thread(&CameraStreamProcessorImpl::doFaceDetectWork, this);
        _doFaceDetectThr.detach();
    }
    _isFaceDetectProcessing = true;
    _faceDetectReady = false;
    _max_process_frame_num = -1;
    logVerbose("X");
};

void CameraStreamProcessorImpl::startGenFaceLandMark() {
    logVerbose("E");

    if (!_doGenFaceLandMark) {
        _doGenFaceLandMark = true;
        _isGenFaceLandMarkProcessing = true;
        _doGenFaceLandMarkThr = std::thread(&CameraStreamProcessorImpl::doGenFaceLandMark, this);
        _doGenFaceLandMarkThr.detach();
    }
    logVerbose("X");
};

void CameraStreamProcessorImpl::startStreamDepth() {
    logVerbose("E");

    if (!_doStreamDepth) {
        _doStreamDepth = true;
        _isStreamDepthProcessing = true;
        _timeStampM.resize(40);
        _timeStampL.resize(40);
        _timeStampR.resize(40);
        _doStreamDepthThr = std::thread(&CameraStreamProcessorImpl::doStreamDepthWork, this);
        _doStreamDepthThr.detach();
    }
    logVerbose("X");
};

void CameraStreamProcessorImpl::startRecalibration() {
    logVerbose("E");

    if (!_doReCalibration) {
        _isRecalibrationProcessing = true;
        _doReCalibration = true;
        _doReCalibrationThr = std::thread(&CameraStreamProcessorImpl::doReCalibration, this);
        _doReCalibrationThr.detach();
    }
    logVerbose("X");
};

void CameraStreamProcessorImpl::finishProcessing() {
    logVerbose("E");
    resetPrivateParameters();
    logVerbose("X");
};

void CameraStreamProcessorImpl::cancelProcessing() {
    logVerbose("E");
    resetControlFlags();
    logVerbose("X");
};

bool CameraStreamProcessorImpl::isProcessing() {
    return _isProcessing;
};

bool CameraStreamProcessorImpl::isFaceDetectProcessing() {
    return _isFaceDetectProcessing;
};

bool CameraStreamProcessorImpl::isFaceLandMarkProcessing() {
    return _isGenFaceLandMarkProcessing;
};

bool CameraStreamProcessorImpl::isStreamDepthProcessing() {
    return _isStreamDepthProcessing;
};

bool CameraStreamProcessorImpl::isRecalibrationProcessing() {
    return _isRecalibrationProcessing;
};

void CameraStreamProcessorImpl::registerProcessListener(B3DCameraProcessListenerPtr cameraProcessListener) {
    _processListener = cameraProcessListener;
};

// Find the first frame with open eyes
// search up to maxFrames
int selectFirstFrame(const std::string &rootdir, const std::vector<cv::Mat>& inputImages,
        int maxFrames, cv::Ptr<b3di::FaceTracker>& ft, bool& isCancel)
{
    logVerbose("E");
    int n = static_cast<int>(inputImages.size());

    if (n == 0) {
        logWarning("empty color frame store");
        return -1;
    }

    if (maxFrames > n) {
        logWarning("frontal frame selection number %d is not in the range", maxFrames, n);
        return -1;
    }

    if (maxFrames <= 1) {
        logWarning("search frontal frame maxFrame %d, use first frame",maxFrames);
        return 0;
    }

    if(isCancel) return 1;

    // input images must be RGB
    if (inputImages.front().type() != CV_8UC3)
    {
        logWarning("input images must be RGB, current is %d",inputImages.front().type());
        return 0;
    }

    // resize image
    std::vector<cv::Mat> frameStore;
    frameStore.reserve(inputImages.size());
    const int faceDetectHeight = 640;
    double scale = 1.0;
    if (inputImages.front().rows > faceDetectHeight)
    {
        scale = faceDetectHeight * 1.0 / inputImages.front().rows;
    }

    if(isCancel) return 1;

    for (const cv::Mat& im : inputImages)
    {
        if(isCancel) return 1;
        cv::Mat tmpIm;
        if (scale != 1.0 && im.size().width > 0 && im.size().height)
        {
            cv::resize(im, tmpIm, cv::Size(), scale, scale);
            frameStore.push_back(tmpIm);
        }
        else
        {
            tmpIm = im.clone();
            frameStore.push_back(tmpIm);
        }
    }

    SkinDetector sd;
    cv::Rect eyeRects[2];
    double maxRatio = 0;
    //const double passRatio = 0.2;		// ratio of non-skin (eye ball) to total region to be considered as open eyes

    int selectedI = -1;
    //int m = framePoseContainer.getCount();
    for (int i = 0; i < maxFrames; i++) {

        if(isCancel) return 1;

        const cv::Mat img = frameStore[i];

        if (!img.empty()) {
            // Find the eye rects in the first frame
            // Assume the first few frames have no motion so the eye rects remain the same
            //if (!foundFace) {
            // Can't assume every frame are similar as we may have a bad frame
            // Need to detect face on every frame
            {
                int marginX = (int)(img.cols*0.1);
                int marginY = (int)(img.rows*0.1);

                cv::Rect searchRect(marginX, marginY, img.cols - marginX * 2, img.rows - marginY * 2);

                if(isCancel) return 1;

                bool foundFace = false;
                if(ft)
                    ft->findLandmarks(img, 0, false, searchRect);

                if (!foundFace) {
                    logWarning("face not detected in frame %d. Skip it.", i);
                    continue;
                }

                if(isCancel) return 1;

                // Find interior face rect (exclude outer landmarks)
                cv::Rect faceRect = ft->getFaceRect(ft->getLandmarks2d(), true);
                cv::Mat faceRoi = img(faceRect);
                cv::Mat faceYcc;
                cv::cvtColor(faceRoi, faceYcc, cv::COLOR_BGR2YCrCb);

                vector<cv::Mat> channels;
                cv::split(faceYcc, channels);

                int numBins = 256;
                int histSize = numBins;
                float range[] = { 0, 256 };
                const float* histRange = { range };

                bool uniform = true; bool accumulate = false;

                // Find the Ycc ranges for the face
                cv::Vec2i yccRanges[3];
                cv::Size winSize(512, 250);
                int count = faceYcc.cols*faceYcc.rows;
                double discardRatio = 0.02;
                int minThresh = (int)round(count*discardRatio);
                int maxThresh = (int)round(count*(1.0 - discardRatio));

                for (int k = 1; k < 3; k++) {
                    if(isCancel) return 1;
                    cv::Mat hist;
                    calcHist(&channels[k], 1, nullptr, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
                    //normalize(teethHist, teethHist, 0, teethHist.rows, NORM_MINMAX, -1, Mat());
                    b3di::findHistogramMinMax(hist, minThresh, maxThresh, yccRanges[k][0], yccRanges[k][1]);
                }

                // Seems to work better with fixed Y range
                sd.setRange(15, 240, yccRanges[1][0], yccRanges[1][1], yccRanges[2][0], yccRanges[2][1]);

                if(isCancel) return 1;

                b3di::FaceTracker::getEyeRects(ft->getFaceLandmarks(), eyeRects);

                // Grow the eye rects so they should work for other frames (assuming first few frames have almost no motion)
                for (int k = 0; k < 2; k++) {
                    if(isCancel) return 1;
                    // If the first image has closed eyes, the height tends to be smaller
                    // ensure it has some minimum height or it may miss the next frame's eyes
                    if (eyeRects[k].height < eyeRects[k].width / 2)
                        eyeRects[k].height = eyeRects[k].width / 2;
                    eyeRects[k] = b3di::growRect(eyeRects[k], eyeRects[k].width / 4, eyeRects[k].height / 2);
                }
            }

            cv::Mat eyeImgs[2];
            cv::Mat eyeMasks[2];
            double eyeRatios[2];
            for (int j = 0; j < 2; j++) {
                if(isCancel) return 1;
                eyeImgs[j] = img(eyeRects[j]);
                eyeMasks[j] = sd.getSkinMask(eyeImgs[j]);
                b3di::countBlackPixels(eyeMasks[j], eyeRatios[j]);
            }

            // get the min of the two eye ratios
            double theRatio = (eyeRatios[0] + eyeRatios[1]) / 2.0;

            // Otherwise, find one with the max ratio
            if (theRatio > maxRatio) {
                maxRatio = theRatio;
                selectedI = i;
            }
        }
    }

    if(isCancel) return 1;

    if (selectedI >= 0) {
        logInfo("Select frame index %d as the frontal frame",selectedI);
    }
    else {
        logWarning("cannot find a valid frontal frame from the first %d frames", maxFrames);
    }
    logVerbose("X");
    return selectedI;
}

int calculateLMoffset(int keyFrames, vector<FrameTime> timeStampM, vector<FrameTime> timeStampL) {
    logVerbose("E");
    int targetTimeStamp = timeStampM[keyFrames];
    int indexL = keyFrames;
    int mindiff = INT_MAX;
    for(int x = 0 ; x < timeStampL.size() ; x++) {
        int diff = abs((int)(targetTimeStamp - timeStampL[x]));
        if(diff < mindiff) {
            mindiff = diff;
            indexL = x;
        }
    }
    logVerbose("X");
    return indexL - keyFrames;
}

void CameraStreamProcessorImpl::doKeyFrameSelection(int maxFrames)
{
	logVerbose("E");
    b3dd::TLog::setLogFile((_sessiondir + ("/processing.log")).c_str(), true);
    const double startDoKeyFrameSelection = b3di::now_ms();

    while(_doKeyFrameSelect) {
        try {
            if (_colorFrameStore.size() < maxFrames) {
                logVerbose("waiting for color frames");
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                CHECKCANCEL(_doKeyFrameSelect);
                continue;
            } else {
                CHECKCANCEL(_doKeyFrameSelect);
                int keyFrames = selectFirstFrame(_rootdir, _colorFrameStore, maxFrames, _ft, _doKeyFrameSelect);
                keyFrames = keyFrames >= 0 ? keyFrames : 0;
                int offset = calculateLMoffset(keyFrames, _timeStampM, _timeStampL);
                logInfo("offset %d", offset);
                CHECKCANCEL(_doKeyFrameSelect);
                if(_processListener)
                    _processListener->onFrameEnough(_colorFrameStore, keyFrames, offset);
                else
                    logWarning("_processListener is null");
                _doKeyFrameSelect = false;
                _colorFrameStore.clear();
            }
        } catch (cv::Exception &e) {
            B3DNativeProcessError nativehErr;
            const char *err_msg = e.what();
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                    err_msg);
            logError("doKeyFrameSelection with error %s", (nativehErr.debugMessage).c_str());
            if(_processListener)
                _processListener->onError(nativehErr);
            return;
        } catch (...) {
            auto expPtr = std::current_exception();
            B3DNativeProcessError nativehErr;
            try {
                if (expPtr) std::rethrow_exception(expPtr);
            } catch (const std::exception &e) {
                const char *err_msg = e.what();
                nativehErr = B3DNativeProcessError(
                        B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                        err_msg);
                logError("doKeyFrameSelection some error occur %s", (nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                return;
            }
        }
        CHECKCANCEL(_doKeyFrameSelect);
    }

    logInfo("do KeyFrameSelection -- %.2f", b3di::now_ms() - startDoKeyFrameSelection);

    logVerbose("X");
}


// cvt cams to bin
static bool cvtCameraParamsToB3DCalibBin(
        const std::vector<b3di::CameraParams>& cams,
        B3DCalibrationData& calibBin
)
{
    logVerbose("E");
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
    logVerbose("X");
    return true;
}

// run auto recalib
static B3DNativeProcessError recomputeCalibBin(const cv::Mat& imageL, const cv::Mat& imageR,
    const b3di::CameraParams& calibL, const b3di::CameraParams& facCalibR,
    const b3di::CameraParams& calibM, const b3di::CameraParams& localCalibR,
    b3di::CameraParams& newCalibR, B3DCalibrationData& newCalinBin, float& dispErr,
    int maxSearchOffset = 5, int depthCamType = b3di::DepthCamType::DEPTHCAM_B3D4, float errorThr = 0.0f,
    bool debug = false
)
{
    logVerbose("E");
    // check if images are empty
    if (imageL.empty() || imageR.empty())
    {
        logError("input images is empty, L : %d, R %d",imageL.empty(),imageR.empty());
        return B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_RECALIBRATION_INPUT_IMAGE_INVALID, "Invalid input image");
    }

    // create auto calib
    b3di::CameraParams camL = calibL;
    // for manual recalibration errorThr == 0, use facCalibR as input
    // for auto recalibration errorThr != 0, use local facCalibR as input
    b3di::CameraParams camR = errorThr == 0 ? facCalibR : localCalibR;
    bool calibChange = false;

    B3D4AutoCalib autoCalib;
    const auto depthSDKCamType = b3dd::DepthCameraType::DEPTHCAM_B3D4;
    cv::Rect2f roi(0.1f, 0.15f, 0.8f, 0.7f);

    auto t0 = b3di::now_ms();
    logVerbose("Start Recalibration Process");

    // check compute disp error with local camR if errorThr is valid
    if (errorThr > 0)
    {
        // compute disp errors with local calib data and fac calib data
        float localCalibDispErr = -1.0f, facCalibDispErr = -1.0f;
        int localCalibValidFeaturePoints = 0, facCalibValidFeaturePoints = 0;
        bool ret = autoCalib.computeDispError(imageL, imageR, camL, localCalibR,
            b3dd::DEPTHCAM_B3D4, roi, localCalibDispErr, localCalibValidFeaturePoints);
        ret &= autoCalib.computeDispError(imageL, imageR, camL, facCalibR,
            b3dd::DEPTHCAM_B3D4, roi, facCalibDispErr, facCalibValidFeaturePoints);

        logVerbose("Auto Recalibration localDispError[%.2f] facDispError[%.2f]",
                localCalibDispErr, facCalibDispErr);
        logVerbose("Auto Recalibration localCalibValidFeaturePoints[%d] facCalibValidFeaturePoints[%d]",
                   localCalibValidFeaturePoints, facCalibValidFeaturePoints);

        // compare the error and set params for Auto Recalibration process
        // use the one with smaller error for Auto Recalibration process
        float computeDispError = localCalibDispErr;
        const bool useFacCalibData = localCalibDispErr > errorThr && facCalibDispErr < localCalibDispErr;

        if (useFacCalibData)
        {
            camR = facCalibR;
            computeDispError = facCalibDispErr;
            dispErr = facCalibDispErr;
            logVerbose("Run Auto Recalibration -- use fac calib data as input");
        } else {
            dispErr = localCalibDispErr;
            logVerbose("Run Auto Recalibration -- use local calib data as input");
        }

        // if disp error > error thr
        // run auto recalibration with factory calibration files
        if (ret && computeDispError > errorThr)
        {
            if (autoCalib.recalibrate(imageL, imageR, camL, camR, calibChange, depthSDKCamType,
                                      maxSearchOffset, roi, depthCamType, errorThr))
            {
                logVerbose("Run Auto Recalibration -- %.0fms", b3di::now_ms() - t0);
            } else {
                logWarning("Run Auto Recalibration failed");
            }
        }

        // always set calibChange = true if facCalibDispErr < localCalibDispErr
        // so that this scan session will use fac calib data
        // or the Auto Recalib result based on fac calib data for processing
        if (useFacCalibData)
        {
            calibChange = true;
            logVerbose("Run Auto Recalibration -- use fac calib data for this scan session");
        }
    } else {
        // run recalibrate directly if there is no valid errorThr
        if (autoCalib.recalibrate(imageL, imageR, camL, camR, calibChange, depthSDKCamType,
                                  maxSearchOffset, roi, depthCamType, errorThr))
        {
            logVerbose("Run Recalibration -- %.0fms", b3di::now_ms() - t0);
        } else {
            logWarning("Run Recalibration failed");
        }
    }

    // save recalib data if calibChange
    if (calibChange)
    {
        newCalibR = camR;
        std::vector<b3di::CameraParams> camParams{
            calibL, camR, calibM
        };
        cvtCameraParamsToB3DCalibBin(camParams, newCalinBin);

        // save debug data
        if(debug) {
            const std::string recalibDir = "/sdcard/Bellus3d/Arc/ArcClient/debug/recalibration/";
            cv::imwrite(recalibDir + "RecalibrationL.png", imageL);
            cv::imwrite(recalibDir + "RecalibrationR.png", imageR);
            calibL.writeParams(recalibDir + "originalLeftCam.yml");
            facCalibR.writeParams(recalibDir + "originalRightCam.yml");
            camR.writeParams(recalibDir + "reCalibRightCam.yml");
            writeB3DCalibrationData(recalibDir, "reCalibData.bin", newCalinBin);
        }

        return B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_RECALIBRATION_CALIB_NO_ERROR, "no error");
    }
    else
    {
        return B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_RECALIBRATION_CALIB_NO_CHANGE, "no need to change calib file");
    }
    logVerbose("X");
}


void CameraStreamProcessorImpl::doReCalibration()
{
    logVerbose("E");
    while(_doReCalibration || _isRecalibrationProcessing) {
        CHECKCANCEL(_doReCalibration);
        if(_stereoComposer.begin()->second.first.empty() ||
           _stereoComposer.begin()->second.second.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        else {
            CHECKCANCEL(_doReCalibration);
            try {
                // read data from fac data dir
                std::vector<b3di::CameraParams> cams(3);
                cams[0].loadParams(_facDataDir + "leftCam.yml");
                cams[1].loadParams(_facDataDir + "rightCam.yml");
                cams[2].loadParams(_facDataDir + "midCam.yml");
                b3di::CameraParams localCamR;
                localCamR.loadParams(_calibrationdir + "rightCam.yml");

                // get IR images from image store
                cv::Mat irL = _stereoComposer.begin()->second.first.clone();
                cv::Mat irR = _stereoComposer.begin()->second.second.clone();

                b3di::CameraParams newCam;
                B3DCalibrationData newCalibBin;
                float calibDispErr = -1.0f;

                logVerbose("Run Auto Recalibratiton");
                CHECKCANCEL(_doReCalibration);
                B3DNativeProcessError recomputeSucceed = recomputeCalibBin(
                        irL, irR,
                        cams[0], cams[1],
                        cams[2], localCamR,
                        newCam, newCalibBin,
                        calibDispErr,
                        5, b3di::DepthCamType::DEPTHCAM_B3D4, 0.0f,
                        _debugFlag
                );
                CHECKCANCEL(_doReCalibration);
                if (recomputeSucceed.errorCode == B3DNativeProcessError::B3D_RECALIBRATION_CALIB_NO_ERROR) {
                    logInfo("update calib files");
                    // export data to _calibrationdir
                    newCam.writeParams(_calibrationdir + "rightCam.yml", false);
                    writeB3DCalibrationData(_calibrationdir, "b3dCalibData.bin", newCalibBin);
                    CameraCalibExtPtr cameraCalibExtPtr = std::make_shared<b3dd::CameraCalibExt>();
                    const B3DCalibrationData *calibBinPtr = &newCalibBin;
                    cameraCalibExtPtr->cacheCalibrationData(
                            (unsigned char *) calibBinPtr,
                            _calibrationdir + "calib.data",
                            DepthCameraType::DEPTHCAM_B3D4
                    );
                    _cameraCalibExtPtr->loadFromFile(_calibrationdir + "calib.data");
                    _depthProcessor.loadCalibrationData(_cameraCalibExtPtr);
                }
                CHECKCANCEL(_doReCalibration);
                if (_isRecalibrationProcessing) {
                    _isRecalibrationProcessing = false;
                    _doReCalibration = false;
                    if(_processListener)
                        _processListener->onRecalibrationDone(recomputeSucceed, calibDispErr);
                } else {
                    _reCalibrationReady = true;
                    _doReCalibration = false;
                }
            } catch (cv::Exception &e) {
                B3DNativeProcessError nativehErr;
                const char *err_msg = e.what();
                nativehErr = B3DNativeProcessError(
                        B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                        err_msg);
                logError("recalibration with error %s", (nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                return;
            } catch (...) {
                auto expPtr = std::current_exception();
                B3DNativeProcessError nativehErr;
                try {
                    if (expPtr) std::rethrow_exception(expPtr);
                } catch (const std::exception &e) {
                    const char *err_msg = e.what();
                    nativehErr = B3DNativeProcessError(
                            B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                            err_msg);
                    logError("some error occur %s", (nativehErr.debugMessage).c_str());
                    if(_processListener)
                        _processListener->onError(nativehErr);
                    return;
                }
            }
        }
        CHECKCANCEL(_doReCalibration);
    }

    if(_processListener)
        _processListener->onProcessFinished(__func__);
    logVerbose("X");
}

void CameraStreamProcessorImpl::doDepthWork() {
    b3dd::TLog::setLogFile((_sessiondir + ("/processing.log")).c_str(), true);
    logVerbose("E");

    const double startDoDepthWork = b3di::now_ms();

    int frame_counter = 0;

    while(_doDepth) {
        logVerbose("_stereoFrameStore %d", _stereoFrameStore.empty());
        CHECKCANCEL(_doDepth);
        if (_stereoFrameStore.empty() ) {
            logVerbose("waiting for stereo frames");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        else {
            CHECKCANCEL(_doDepth);
            if(_stereoFrameStore.empty() || _stereoFrameScale.empty() || !_reCalibrationReady ) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            try {
                logVerbose("start");
                B3DImage ImageL, ImageR, outImage;
                bool computetesult = false;
                auto front = _stereoFrameStore.front();
                const float imageScale = _stereoFrameScale.front();
                _stereoFrameStore.pop();
                _stereoFrameScale.pop();
                CHECKCANCEL(_doDepth);
                if(frame_counter == 0 ) {
                    // do recalibration check
                    // read data from fac data dir
                    std::vector<b3di::CameraParams> cams(3);
                    cams[0].loadParams(_facDataDir + "leftCam.yml");
                    cams[1].loadParams(_facDataDir + "rightCam.yml");
                    cams[2].loadParams(_facDataDir + "midCam.yml");
                    b3di::CameraParams localCamR;
                    localCamR.loadParams(_calibrationdir + "rightCam.yml");

                    // get IR images from image store
                    cv::Mat irL, irR;
                    B3DImageToCvMat(front.first, irL);
                    B3DImageToCvMat(front.second, irR);

                    b3di::CameraParams newCam;
                    B3DCalibrationData newCalibBin;
                    float calibDispErr = -1.0f;
                    int maxSearchOffset = _needManualRecalib ? 5 : 1;
                    float errorThr = _needManualRecalib ? 0.0f : 0.7f;

                    CHECKCANCEL(_doDepth);
                    B3DNativeProcessError recomputeSucceed = recomputeCalibBin(
                            irL, irR,
                            cams[0], cams[1], cams[2], localCamR,
                            newCam, newCalibBin,
                            calibDispErr,
                            maxSearchOffset, b3di::DepthCamType::DEPTHCAM_B3D4, errorThr,
                            _debugFlag
                    );
                    CHECKCANCEL(_doDepth);
                    if (recomputeSucceed.errorCode == B3DNativeProcessError::B3D_RECALIBRATION_CALIB_NO_ERROR ) {
                        if(!_needManualRecalib) {
                            logInfo("Auto Recalibration succeed");
                            // export data to _calibrationdir
                            // for auto recalibration, only save files to as tmp file
                            const std::string tmpRightCamFile = "rightCam_tmp.yml";
                            const std::string tmpCalibBinFile = "b3dCalibData_tmp.bin";
                            const std::string tmpCalibDataFile = "calib_tmp.data";
                            newCam.writeParams(_calibrationdir + tmpRightCamFile, false);
                            writeB3DCalibrationData(_calibrationdir, tmpCalibBinFile, newCalibBin);
                            CameraCalibExtPtr cameraCalibExtPtr = std::make_shared<b3dd::CameraCalibExt>();
                            const B3DCalibrationData *calibBinPtr = &newCalibBin;
                            cameraCalibExtPtr->cacheCalibrationData(
                                    (unsigned char *) calibBinPtr,
                                    _calibrationdir + tmpCalibDataFile,
                                    DepthCameraType::DEPTHCAM_B3D4
                            );
                            _cameraCalibExtPtr->loadFromFile(_calibrationdir + tmpCalibDataFile);
                            _depthProcessor.loadCalibrationData(_cameraCalibExtPtr);
                        } else {
                            logInfo("Manual Recalibration succeed");
                            // export data to _calibrationdir
                            newCam.writeParams(_calibrationdir + "rightCam.yml", false);
                            writeB3DCalibrationData(_calibrationdir, "b3dCalibData.bin", newCalibBin);
                            CameraCalibExtPtr cameraCalibExtPtr = std::make_shared<b3dd::CameraCalibExt>();
                            const B3DCalibrationData *calibBinPtr = &newCalibBin;
                            cameraCalibExtPtr->cacheCalibrationData(
                                    (unsigned char *) calibBinPtr,
                                    _calibrationdir + "calib.data",
                                    DepthCameraType::DEPTHCAM_B3D4
                            );
                            _cameraCalibExtPtr->loadFromFile(_calibrationdir + "calib.data");
                            _depthProcessor.loadCalibrationData(_cameraCalibExtPtr);
                        }
                    } else if(recomputeSucceed.errorCode == B3DNativeProcessError::B3D_RECALIBRATION_CALIB_NO_CHANGE){
//                        //do nothing
                        logInfo("Auto Recalibration no need to change");
                    } else {
                        logError("Skip Auto Recalibration, error code %d",recomputeSucceed.errorCode);
                    }

                    if(_processListener)
                        _processListener->onRecalibrationDone(recomputeSucceed, calibDispErr);

                    logInfo("Auto Recalibration: Fac cy[%.2f] Local cy[%.2f] autoRecalib cy[%.2f]",
                            cams[1].cy(), localCamR.cy(), newCam.cy());
                }
                CHECKCANCEL(_doDepth);
                if (imageScale == 1.0f)
                {
                    _depthConfigPtr->optimizeMode = OPTIMIZE_FULL_RES;
                }
                else if (imageScale == 0.75f)
                {
                    _depthConfigPtr->optimizeMode = OPTIMIZE_HALF_RES;
                }
                else if (imageScale == 0.5f)
                {
                    _depthConfigPtr->optimizeMode = OPTIMIZE_QUARTER_RES;
                }

                _depthConfigPtr->depthScale = imageScale;

                if (_depthFrameStore.empty()) // first depth frame uses IRThres
                    _depthConfigPtr->useIRThres = true;
                else
                    _depthConfigPtr->useIRThres = false;

                _depthProcessor.updateProcessorSettingsExt(_depthConfigPtr);

                logVerbose("Input image size (%d, %d) (%d, %d), scale %.2f",
                        front.first.cols(), front.first.rows(), front.second.cols(), front.second.rows(),
                        imageScale);
                CHECKCANCEL(_doDepth);
                computetesult = _depthProcessor.computeDepth(front.first, front.second, outImage);
                CHECKCANCEL(_doDepth);
                logVerbose("Output depth size (%d, %d)",outImage.cols(), outImage.rows());

                logInfo("computeDepth (state, count, store size) -> (%d, %d, %d)", computetesult, frame_counter, _depthFrameStore.size());
                if (computetesult) {
                    B3DImage showDepth;
                    cv::Mat depthImage;
                    B3DImageToCvMat(outImage, depthImage);

                    const float targetDepthImageScale = 0.75f;
                    if (imageScale != targetDepthImageScale) {
                        cv::resize(depthImage, depthImage, cv::Size(),
                                   targetDepthImageScale / imageScale, targetDepthImageScale / imageScale, cv::INTER_NEAREST);
                    }
                    CHECKCANCEL(_doDepth);
                    logVerbose("depthFrame size (%d, %d)", depthImage.cols, depthImage.rows);

                    if(_debugFlag)
                        saveDepth(_sessiondir, frame_counter, depthImage);

                    _depthFrameStore.push_back(depthImage);

                    frame_counter++;
                    _depthProcessor.printTimingStatistics();
                } else {
                    logError("Compute depth error");
                    finishProcessing();
                }
            } catch (b3dd::B3D_DEPTH_COMPUTATION_ERROR& err) {

                B3DNativeProcessError nativehErr;
                switch(err) {
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_CONFIG_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_CONFIG_INVALID,"Invalid depth config");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_IMAGE_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_IMAGE_INVALID,"Invalid depth image");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_ROI_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_ROI_INVALID,"Invalid depth ROI");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INTPUT_CALIBATION_DATA_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_CALIBATION_DATA_INVALID,"Invalid depth calibration data");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INTPUT_RECT_MAP_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_RECT_MAP_INVALID,"Invalid rect map");
                        break;
                    default:
                        break;
                }

                logError("compute depth with error %s",(nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                return;
            } catch( cv::Exception& e ) {
                B3DNativeProcessError nativehErr;
                const char* err_msg = e.what();
                nativehErr = B3DNativeProcessError(
                        B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                        err_msg);
                logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                return;
            } catch (...) {
                auto expPtr = std::current_exception();
                B3DNativeProcessError nativehErr;
                try {
                    if(expPtr) std::rethrow_exception(expPtr);
                } catch(const std::exception& e) {
                    const char* err_msg = e.what();
                    nativehErr = B3DNativeProcessError(
                            B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                            err_msg);
                    logError("some error occur %s",(nativehErr.debugMessage).c_str());
                    if(_processListener)
                        _processListener->onError(nativehErr);
                    return;
                }
            }
        }

        if (_depthFrameStore.size() == _max_process_frame_num) {
            if(_processListener)
                _processListener->onProcessDone(_depthFrameStore,faceRect);
            else
                logWarning("_processListener is null");
            _doDepth = false;
        }
    }

    if(_processListener)
        _processListener->onProcessFinished(__func__);

    logInfo("do depth work -- %.2f", b3di::now_ms() - startDoDepthWork);
    logVerbose("X");
}


static cv::Rect projectFaceRectToDepthSpace(const cv::Rect& faceRect, float faceDistance,
                                            const b3di::CameraParams& initCam, const b3di::CameraParams& depthCam)
{
    logVerbose("E");
    // face rect data
    const float x = static_cast<float>(faceRect.x);
    const float y = static_cast<float>(faceRect.y);
    const int width = faceRect.width;
    const int height = faceRect.height;

    // face rect points
    std::vector<cv::Point3f> faceRectPoint{
            cv::Point3f(x, y, faceDistance),
            cv::Point3f(x + width, y, faceDistance),
            cv::Point3f(x + width, y + height, faceDistance),
            cv::Point3f(x, y + height, faceDistance),
    };

    // init camera params
    const float fx = static_cast<float>(initCam.fx());
    const float fy = static_cast<float>(initCam.fy());
    const float cx = static_cast<float>(initCam.cx());
    const float cy = static_cast<float>(initCam.cy());

    // depth camera params
    const float depthFx = static_cast<float>(depthCam.fx());
    const float depthFy = static_cast<float>(depthCam.fy());
    const float depthCx = static_cast<float>(depthCam.cx());
    const float depthCy = static_cast<float>(depthCam.cy());

    // project rect points to init cam space
    // transfter 3d pts to depthCam space
    trimesh::xform xf = depthCam.getCameraToWorldXform()
                        * trimesh::inv(initCam.getWorldToCameraXform());

    std::vector<float> ptX, ptY;
    for (auto& pt : faceRectPoint)
    {
        float x = pt.x;
        float y = pt.y;
        float z = pt.z;

        x = z * (x - cx) / fx;
        y = z * (y - cy) / fy;

        // transfer points to depth cam space
        trimesh::vec3 newPt = xf*trimesh::vec3(x, y, z);

        x = newPt[0];
        y = newPt[1];
        z = newPt[2];

        ptX.push_back(x / z * depthFx + depthCx);
        ptY.push_back(y / z * depthFy + depthCy);
    }

    std::sort(ptX.begin(), ptX.end());
    std::sort(ptY.begin(), ptY.end());

    int minX = static_cast<int>(ptX.front() + 0.5f);
    int maxX = static_cast<int>(ptX.back() + 0.5f);
    int minY = static_cast<int>(ptY.front() + 0.5f);
    int maxY = static_cast<int>(ptY.back() + 0.5f);

    const int cols = depthCam.getImageSize().width;
    const int rows = depthCam.getImageSize().height;

    minX = minX > 0 ? minX : 0;
    maxX = maxX < cols ? maxX : cols - 1;
    minY = minY > 0 ? minY : 0;
    maxY = maxY < rows ? maxY : rows - 1;
    logVerbose("X");
    return cv::Rect(
            minX, minY,
            maxX - minX,
            maxY - minY
    );
}


void CameraStreamProcessorImpl::doFaceDetectWork() {
    logVerbose("E");
    const double doFaceDetectWork = b3di::now_ms();
    int frame_counter = 0;

    int lmCount = b3di::FaceTracker::LM_COUNT_ASM;

    // load calib data for head pose track
    const std::string colorCamPath = _calibrationdir + "midCam.yml";
    const std::string depthCamPath = _calibrationdir + "depthCam.yml";

    b3di::CameraParams colorCam, depthCam;
    try {
        colorCam.loadParams(colorCamPath);
        depthCam.loadParams(depthCamPath);
    } catch( cv::Exception& e ) {
        B3DNativeProcessError nativehErr;
        const char* err_msg = e.what();
        if( e.code == cv::Error::StsNullPtr ) {
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_FILE_MISSING,
                    err_msg);
        } else {
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                    err_msg);
        }
        logError("loadParams error %s", (nativehErr.debugMessage).c_str());
        if(_processListener)
            _processListener->onError(nativehErr);
        return;
    } catch (...) {
        auto expPtr = std::current_exception();
        B3DNativeProcessError nativehErr;
        try {
            if(expPtr) std::rethrow_exception(expPtr);
        } catch(const std::exception& e) {
            const char* err_msg = e.what();
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                    err_msg);
            logError("some error occur when loadParams %s",(nativehErr.debugMessage).c_str());
            if(_processListener)
                _processListener->onError(nativehErr);
            return;
        }
    }

    // head track image max height
    const int FACE_TRACK_MAX_IMAGE_HEIGHT = 640;
    // data buffer for t x, y, z and r x, y, z
    const int FACE_TRACK_BUFFER_SIZE = 1;

    std::vector<std::vector<float>> headPoseDataBuffer(6, std::vector<float>(FACE_TRACK_BUFFER_SIZE, 0.0f));

    int frameCnt = 0;
    const float minFaceDistance = 150.0f;
    const float maxFaceDistance = 600.0f;

    float faceDistance = 350.0f;

    // record face distance tracking
    const std::string tmpFaceTrackingDataDir = "/sdcard/Bellus3d/ARC/ArcClient/faceTracking_tmp/";

    // clear face tracking tmp data
    if (cv::utils::fs::isDirectory(tmpFaceTrackingDataDir))
    {
        cv::utils::fs::remove_all(tmpFaceTrackingDataDir);
    }
    cv::utils::fs::createDirectory(tmpFaceTrackingDataDir);

    std::ofstream faceTrackingFile;
    faceTrackingFile.open(tmpFaceTrackingDataDir + "faceTracking_tmp.csv");

    if (faceTrackingFile.is_open())
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        faceTrackingFile << std::put_time(&tm, "%d-%m-%Y %H-%M-%S\n");
        faceTrackingFile << "frameCnt, " << "validDepthPixelCnt, "
                         << "facePosition.x, "
                         << "facePosition.y, "
                         << "facePosition.z, "
                         << "faceRect.x, "
                         << "faceRect.y, "
                         << "faceRect.width, "
                         << "faceRect.height\n";
    }

    // record previous distance computation result
    float faceDistancePre = 0.0f;
    cv::Mat depthImagePre;

    while(_doFaceDetect) {
        logVerbose("_stereoFrameStore %d", _stereoFrameStore.empty());
        CHECKCANCEL(_doFaceDetect);
        // always do face detection first
        if (!_stereoFrameStore.empty() && !_colorImageStore.empty() && !_stereoFrameScale.empty()) {
            try {
                logVerbose("start");
                const double startFastFaceTrack = b3di::now_ms();
                ++frameCnt;
                cv::Mat depthImage;

                cv::Vec3f headPosition(0.0f, 0.0f, 0.0f); // translation
                cv::Vec3f headRotation(0.0f, 0.0f, 0.0f); // rotation

                B3DImage ImageL, ImageR, outImage;
                bool computeResult = false;
                auto front = _stereoFrameStore.back();
                const float imageScale = _stereoFrameScale.back();
                cv::Mat colorImage = _colorImageStore.back();

                // clear queue
                std::queue<std::pair<B3DImage, B3DImage>> emptyStereoFrameStore;
                std::queue<float> emptyStereoFrameScale;
                std::queue<cv::Mat> emptyColorImageStore;
                std::swap(_stereoFrameStore, emptyStereoFrameStore);
                std::swap(_stereoFrameScale, emptyStereoFrameScale);
                std::swap(_colorImageStore, emptyColorImageStore);

                // do face rect detection
                double faceTrackImageScale = FACE_TRACK_MAX_IMAGE_HEIGHT
                                             / static_cast<double>(colorImage.rows);
                cv::Mat faceTrackImage;
                if (faceTrackImageScale == 1.0) {
                    faceTrackImage = colorImage.clone();
                } else {
                    cv::resize(colorImage, faceTrackImage, cv::Size(),
                               faceTrackImageScale, faceTrackImageScale);
                }
                const int faceTrackImageWidth = faceTrackImage.cols;
                const int faceTrackImageHeight = faceTrackImage.rows;

                if (faceTrackImage.channels() != 1)
                    cv::cvtColor(faceTrackImage, faceTrackImage, cv::COLOR_BGR2GRAY);

                const b3di::DepthCamProps depthCamProps =
                        b3di::DEPTHCAM_PROPS[b3di::DepthCamType::DEPTHCAM_B3D4];

                float faceSearchRegionRatioW
                        = (float) depthCamProps.faceDetectionWindowWidthRatio;
                float faceSearchRegionRatioH
                        = (float) depthCamProps.faceDetectionWindowHeightRatio;

                float topMargin = (1.0f - faceSearchRegionRatioH) * 0.5f;
                float botMargin = topMargin;

                const int xMarginPix = static_cast<int>((1 - faceSearchRegionRatioW)
                                                        / 2 * faceTrackImage.cols + 0.5f);
                const int yMarginPix = static_cast<int>(topMargin * faceTrackImage.rows + 0.5f);
                const auto startFaceRectDetection = b3di::now_ms();
                logVerbose("Head Pose Tracking - search face rect");
                logVerbose("Head Pose Tracking - faceTrackImage (%d, %d) xMargin %d yMargin %d",
                           faceTrackImage.cols, faceTrackImage.rows, xMarginPix, yMarginPix);
                const cv::Rect faceRect = b3di::FaceTracker::findFaceRect(faceTrackImage, xMarginPix,
                                                                          yMarginPix);
                logVerbose("Head Pose Tracking - face rect(%d, %d, %d, %d) -- %.2fms",
                           faceRect.x, faceRect.y, faceRect.width, faceRect.height,
                           b3di::now_ms() - startFaceRectDetection);

                if (colorCam.getImageSize() != faceTrackImage.size())
                    colorCam.setImageSize(faceTrackImage.size());

                const float fx = static_cast<float>(colorCam.fx());
                const float fy = static_cast<float>(colorCam.fy());
                const float cx = static_cast<float>(colorCam.cx());
                const float cy = static_cast<float>(colorCam.cy());

                CHECKCANCEL(_doFaceDetect);

                int validDepthPixelCnt = 0;
                // face rect detection succeed
                if (faceRect != cv::Rect())
                {
                    const auto startComputeFaceDistance = b3di::now_ms();

                    cv::Rect faceRectDepth =
                            projectFaceRectToDepthSpace(faceRect, minFaceDistance, colorCam, depthCam);

                    _depthConfigPtr->optimizeMode = imageScale == 0.75f ? OPTIMIZE_HALF_RES :
                                                    imageScale == 0.5f ? OPTIMIZE_QUARTER_RES :
                                                    imageScale == 0.25f ? OPTIMIZE_QQUARTER_RES : OPTIMIZE_FULL_RES;
                    _depthConfigPtr->depthRegistration = REGISTER_L;
                    _depthConfigPtr->sceneMode = NORMAL;
                    _depthConfigPtr->depthScale = imageScale;
                    _depthConfigPtr->depthUnit = 0.02f;

                    _depthConfigPtr->useIRThres         = true;
                    _depthConfigPtr->useForegroundMask  = true;
                    _depthConfigPtr->useDepthRange      = true;
                    _depthConfigPtr->useTwoDispMaps     = true;
                    _depthConfigPtr->reduceDispBias     = false;
                    _depthConfigPtr->ROI_L =
                            ROI(0, faceRectDepth.y, depthCam.getImageSize().width, faceRectDepth.height);

                    _depthProcessor.updateProcessorSettingsExt(_depthConfigPtr);

                    logVerbose("Input image size (%d, %d) (%d, %d), scale %.2f",
                               front.first.cols(), front.first.rows(), front.second.cols(), front.second.rows(),
                               imageScale);

                    CHECKCANCEL(_doFaceDetect);
                    const auto startComputeDepth = b3di::now_ms();
                    computeResult = _depthProcessor.computeDepth(front.first, front.second, outImage);
                    CHECKCANCEL(_doFaceDetect);
                    logVerbose("Head Pose Tracking - compute depth %.2fms", b3di::now_ms() - startComputeDepth);
                    _depthProcessor.printTimingStatistics();

                    if (computeResult) {
//                        cv::Mat depthImage;
                        B3DImageToCvMat(outImage, depthImage);

                        // resize camera params
                        b3di::CameraParams outputCam = depthCam;
                        if (depthCam.getImageSize() != depthImage.size())
                            outputCam.setImageSize(depthImage.size());

                        // porject key depth frame to key color image space
                        b3di::PointCloud pc;
                        pc.create(depthImage, outputCam.getIntrinsicMat());

                        // re-project 3D points back to M cam image space
                        trimesh::xform depthCamToKeyColorCamXf = colorCam.getWorldToCameraXform();
                        pc.transformPoints(depthCamToKeyColorCamXf);

                        cv::Mat depthInColorSpace;
                        pc.projectPointsToDepth(
                                colorCam.getIntrinsicMat(),
                                colorCam.getDistCoeffs(),
                                colorCam.getImageSize(),
                                depthInColorSpace
                        );

                        // target region for face distance measurement
                        // center region, 25% of the pixels
                        const int c0 = faceRect.x + faceRect.width / 2 - faceRect.width / 4;
                        const int c1 = faceRect.x + faceRect.width / 2 + faceRect.width / 4;
                        const int r0 = faceRect.y + faceRect.height / 2 - faceRect.height / 4;
                        const int r1 = faceRect.y + faceRect.height / 2 + faceRect.height / 4;
                        std::vector<float> distances;
                        distances.reserve(faceRect.width / 2 * faceRect.height / 2);
                        float sum = 0;
                        const float depthRes = 0.02f;
                        for (int r = r0; r != r1; ++r)
                        {
                            const ushort* ptr = depthInColorSpace.ptr<ushort>(r, c0);
                            for (int c = c0; c != c1; ++c)
                            {
                                const ushort val = *ptr++;
                                if (val)
                                {
                                    float dist = val * depthRes;
                                    sum += dist;
                                    ++validDepthPixelCnt;
                                    distances.push_back(dist);
                                }
                            }
                        }

                        if (validDepthPixelCnt)
                        {
                            std::sort(distances.begin(), distances.end());
                            const float distance = distances[distances.size() / 2];
                            if (distance >= minFaceDistance && distance <= maxFaceDistance)
                            {
                                faceDistance = distance;
                                logInfo("Head Pose Tracking [%d] - face distance %.2fmm -- %.0fms",
                                        frameCnt, faceDistance, b3di::now_ms() - startComputeFaceDistance);
                            } else {
                                if(distance < minFaceDistance) faceDistance = 0.0;
                                else if (distance > maxFaceDistance) faceDistance = 999.0;
                                logVerbose("Head Pose Tracking [%d] - face distance out of range %.2fmm", frameCnt, distance);
                            }
                        } else {

                            if (_debugFlag)
                            {
                                cv::Mat tmpImageL, tmpImageR;
                                B3DImageToCvMat(front.first, tmpImageL);
                                B3DImageToCvMat(front.second, tmpImageR);
                                cv::imwrite(tmpFaceTrackingDataDir + "faceTracking_" + std::to_string(frameCnt) + "_L.png", tmpImageL);
                                cv::imwrite(tmpFaceTrackingDataDir + "faceTracking_" + std::to_string(frameCnt) + "_R.png", tmpImageR);
                                cv::imwrite(tmpFaceTrackingDataDir + "faceTracking_" + std::to_string(frameCnt) + "_D.png", depthImage);
                            }

                            faceTrackingFile << frameCnt << ", no valid depth pixel found\n";
                            logError("Head Pose Tracking [%d] - fail to compute face distance", frameCnt);
                        }
                        CHECKCANCEL(_doFaceDetect);
                    }  else {
                        if (!computeResult)
                            logError("Compute depth error");
                        if (faceRect == cv::Rect())
                            logError("Fail to detect face");
                    }

                    if (faceDistance >= minFaceDistance && faceDistance <= maxFaceDistance)
                    {
                        cv::Point faceCenter(
                                faceRect.x + faceRect.width / 2,
                                faceRect.y + faceRect.height / 2
                        );
                        headPosition[0] = static_cast<float>((faceCenter.x - cx) / fx * faceDistance);
                        headPosition[1] = static_cast<float>((faceCenter.y - cy) / fy * faceDistance);
                        headPosition[2] = faceDistance;
                    } else {
                        faceTrackingFile << frameCnt << ", invalid face distance = "
                                         << faceDistance << "\n";
                    }
                } else {
                    faceTrackingFile << frameCnt << ", fail to detect face rect\n";
                }
                CHECKCANCEL(_doFaceDetect);
                // normalize face rect
                std::vector<float> normalizedFaceRect{
                    faceRect.x * 1.0f / faceTrackImageWidth,
                    faceRect.y * 1.0f / faceTrackImageHeight,
                    faceRect.width * 1.0f / faceTrackImageWidth,
                    faceRect.height * 1.0f / faceTrackImageHeight
                };

                // update head pose data
                const int ptr = frameCnt % FACE_TRACK_BUFFER_SIZE;

                headPoseDataBuffer[0][ptr] = headPosition[0];
                headPoseDataBuffer[1][ptr] = headPosition[1];
                headPoseDataBuffer[2][ptr] = headPosition[2];
                headPoseDataBuffer[3][ptr] = headRotation[0];
                headPoseDataBuffer[4][ptr] = headRotation[1];
                headPoseDataBuffer[5][ptr] = headRotation[2];

                auto tmpBuffer = headPoseDataBuffer;
                for (auto& buff : tmpBuffer)
                {
                    std::sort(buff.begin(), buff.end());
                }

                // need to pass it to listener
                std::vector<float> outputHeadPoseInfo{
                        tmpBuffer[0][FACE_TRACK_BUFFER_SIZE / 2],
                        tmpBuffer[1][FACE_TRACK_BUFFER_SIZE / 2],
                        tmpBuffer[2][FACE_TRACK_BUFFER_SIZE / 2],
                        tmpBuffer[3][FACE_TRACK_BUFFER_SIZE / 2],
                        tmpBuffer[4][FACE_TRACK_BUFFER_SIZE / 2],
                        tmpBuffer[5][FACE_TRACK_BUFFER_SIZE / 2],
                        normalizedFaceRect[0],
                        normalizedFaceRect[1],
                        normalizedFaceRect[2],
                        normalizedFaceRect[3]
                };
                CHECKCANCEL(_doFaceDetect);
                // if > max or < min , set to _fastTrackFaceDistance, which is 99 or 0
                if(faceDistance < minFaceDistance || faceDistance > maxFaceDistance)
                {
                    outputHeadPoseInfo[0] = 0;
                    outputHeadPoseInfo[1] = 0;
                    outputHeadPoseInfo[2] = faceDistance;
                }

                logInfo("Head Pose Tracking [%d] - tranlation (%.2f, %.2f, %.2f) rotation (%.2f, %.2f, %.2f) faceRect (%.2f, %.2f, %.2f, %.2f)",
                        frameCnt,
                        outputHeadPoseInfo[0],
                        outputHeadPoseInfo[1],
                        outputHeadPoseInfo[2],
                        outputHeadPoseInfo[3],
                        outputHeadPoseInfo[4],
                        outputHeadPoseInfo[5],
                        outputHeadPoseInfo[6],
                        outputHeadPoseInfo[7],
                        outputHeadPoseInfo[8],
                        outputHeadPoseInfo[9]
                );

                // cache face distance tracking data
                if (faceTrackingFile.is_open())
                {
                    faceTrackingFile << frameCnt << ", " << validDepthPixelCnt << ", "
                                     << outputHeadPoseInfo[0] << ", "
                                     << outputHeadPoseInfo[1] << ", "
                                     << outputHeadPoseInfo[2] << ", "
                                     << outputHeadPoseInfo[6] << ", "
                                     << outputHeadPoseInfo[7] << ", "
                                     << outputHeadPoseInfo[8] << ", "
                                     << outputHeadPoseInfo[9] << "\n";
                }

                logInfo("finish fast face tracking -- %.2fms", b3di::now_ms() - startFastFaceTrack);

                const float outlierSize = 100.0f;
                if (faceDistancePre != 0 && faceDistance != 0
                    && std::fabs(faceDistancePre - faceDistance) > outlierSize)
                {
                    if (_debugFlag)
                    {
                        logInfo("Head Pose Tracking -- save depth images for debugging");
                        cv::imwrite(tmpFaceTrackingDataDir + "faceTracking_" + std::to_string(frameCnt - 1) + "_D.png", depthImagePre);
                        cv::imwrite(tmpFaceTrackingDataDir + "faceTracking_" + std::to_string(frameCnt) + "_D.png", depthImage);
                    }

                    faceTrackingFile << frameCnt << ", outlier!!!\n";
                }
                faceDistancePre = faceDistance;

                if (_debugFlag)
                {
                    depthImagePre = depthImage.clone();
                }

                //do face detect
                _depthProcessor.printTimingStatistics();
                if(_processListener)
                    _processListener->onFaceDetectDone(outputHeadPoseInfo);
                else
                    logWarning("_processListener is null");
                std::lock_guard<std::mutex> lock(_gAddFrameLock);
                _faceDetectReady = false;
                CHECKCANCEL(_doFaceDetect);
            } catch (b3dd::B3D_DEPTH_COMPUTATION_ERROR& err) {

                B3DNativeProcessError nativehErr;
                switch (err) {
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_CONFIG_INVALID :
                        nativehErr = B3DNativeProcessError(
                                B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_CONFIG_INVALID,
                                "Invalid depth config");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_IMAGE_INVALID :
                        nativehErr = B3DNativeProcessError(
                                B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_IMAGE_INVALID,
                                "Invalid depth image");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_ROI_INVALID :
                        nativehErr = B3DNativeProcessError(
                                B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_ROI_INVALID,
                                "Invalid depth ROI");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INTPUT_CALIBATION_DATA_INVALID :
                        nativehErr = B3DNativeProcessError(
                                B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_CALIBATION_DATA_INVALID,
                                "Invalid depth calibration data");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INTPUT_RECT_MAP_INVALID :
                        nativehErr = B3DNativeProcessError(
                                B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_RECT_MAP_INVALID,
                                "Invalid rect map");
                        break;
                    default:
                        break;
                }
                logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                return;
            } catch( cv::Exception& e ) {
                B3DNativeProcessError nativehErr;
                const char* err_msg = e.what();
                nativehErr = B3DNativeProcessError(
                        B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                        err_msg);
                logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                return;
            } catch (...) {
                auto expPtr = std::current_exception();
                B3DNativeProcessError nativehErr;
                try {
                    if(expPtr) std::rethrow_exception(expPtr);
                } catch(const std::exception& e) {
                    const char* err_msg = e.what();
                    nativehErr = B3DNativeProcessError(
                            B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                            err_msg);
                    logError("some error occur %s",(nativehErr.debugMessage).c_str());
                    if(_processListener)
                        _processListener->onError(nativehErr);
                    return;
                }
            }
        }
        CHECKCANCEL(_doFaceDetect);
    }

    // close io stream for cache face tracking data
    faceTrackingFile.close();

    if(_processListener)
        _processListener->onProcessFinished(__func__);

    logInfo("do faceDetect work -- %d frames -- %.2fms", frameCnt, b3di::now_ms() - doFaceDetectWork);
    logVerbose("X");
}

void CameraStreamProcessorImpl::doGenFaceLandMark () {
    logVerbose("E");
    while(_doGenFaceLandMark) {
        CHECKCANCEL(_doGenFaceLandMark);
        if(_colorImageStore.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        } else {
            CHECKCANCEL(_doGenFaceLandMark);
            int trackret = -1;
            cv::Mat searchImg = _colorImageStore.front().clone();
            _colorImageStore.pop();

            double startFindLandMark = b3di::now_ms();

            // Detect M cam first image face landmarks
            _ft = b3di::FaceTracker::newFaceTrackerPtr(b3di::FaceTracker::FT_ASM);

            if (!b3di::FaceTrackerASM::initTracker(_rootdir + STASM_FILE_PATH)) {
                B3DNativeProcessError nativehErr =
                        B3DNativeProcessError(
                                B3DNativeProcessError::ErrorCode::B3D_FINDLANDMARK_INITTRACKER_INVALID,
                                "Init Tracker invalid stasm config");
                logError("error %s", (nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                _doGenFaceLandMark = false;
                return;
            }

            int marginX = (int) (searchImg.cols * 0.1);
            int marginY = (int) (searchImg.rows * 0.1);
            CHECKCANCEL(_doGenFaceLandMark);
            cv::Rect searchRect(marginX, marginY, searchImg.cols - marginX * 2,
                                searchImg.rows - marginY * 2);
            bool foundFace = _ft->findLandmarks(searchImg, 0, false, searchRect);
            CHECKCANCEL(_doGenFaceLandMark);
            if (!foundFace) {
                logError("cannot find face in M image size=%d %d",
                         searchImg.cols, searchImg.rows);
                B3DNativeProcessError nativehErr =
                        B3DNativeProcessError(
                                B3DNativeProcessError::ErrorCode::B3D_FINDLANDMARK_FACE_NOT_FOUND,
                                "Can't find face");
                logError("error %s", (nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                _doGenFaceLandMark = false;
                return;
            } else {
                _doGenFaceLandMark = false;
                logInfo("TrackHeadPose -- %.2f", b3di::now_ms() - startFindLandMark);
                if(_processListener)
                    _processListener->onGenFaceLandMarkDone(_ft);
                else
                    logWarning("_processListener is null");
            }
        }
        CHECKCANCEL(_doGenFaceLandMark);
    }
    logVerbose("X");
}

void CameraStreamProcessorImpl::doStreamDepthWork() {
    logVerbose("E");

    const double startStreamDepthWork = b3di::now_ms();

    int framecounter = 0;
#ifdef ENCODEPNG
    vector<uchar> pngBuff;
    vector<int> pngParam = vector<int>(2);
#endif
    while(_doStreamDepth) {
        logVerbose("_stereoFrameStore %d", _stereoFrameStore.empty());
        CHECKCANCEL(_doStreamDepth);
        if (_stereoFrameStore.empty() ) {
            logVerbose("waiting for stereo frames");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        else {
            CHECKCANCEL(_doStreamDepth);
            if(_stereoFrameStore.empty() ||
              _stereoFrameScale.empty() ||
              _stereoLFrameTimeStamp.empty() ||
              !_reCalibrationReady ) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            try {
                logVerbose("start");
                B3DImage ImageL, ImageR, outImage;
                bool computetesult = false;
                auto front = _stereoFrameStore.front();
                const float imageScale = _stereoFrameScale.front();
                const long timeStampL = _stereoLFrameTimeStamp.front();

                _stereoFrameStore.pop();
                _stereoFrameScale.pop();
                _stereoLFrameTimeStamp.pop();
                CHECKCANCEL(_doStreamDepth);
                if(framecounter == 0 ) {
                    // do recalibration check
                    // read data from fac data dir
                    std::vector<b3di::CameraParams> cams(3);
                    cams[0].loadParams(_facDataDir + "leftCam.yml");
                    cams[1].loadParams(_facDataDir + "rightCam.yml");
                    cams[2].loadParams(_facDataDir + "midCam.yml");
                    b3di::CameraParams localCamR;
                    localCamR.loadParams(_calibrationdir + "rightCam.yml");

                    // get IR images from image store
                    cv::Mat irL, irR;
                    B3DImageToCvMat(front.first, irL);
                    B3DImageToCvMat(front.second, irR);

                    b3di::CameraParams newCam;
                    B3DCalibrationData newCalibBin;
                    float calibDispErr = -1.0f;
                    int maxSearchOffset = _needManualRecalib ? 5 : 1;
                    float errorThr = _needManualRecalib ? 0.0f : 0.7f;

                    CHECKCANCEL(_doStreamDepth);
                    B3DNativeProcessError recomputeSucceed = recomputeCalibBin(
                            irL, irR,
                            cams[0], cams[1], cams[2], localCamR,
                            newCam, newCalibBin,
                            calibDispErr,
                            maxSearchOffset, b3di::DepthCamType::DEPTHCAM_B3D4, errorThr,
                            _debugFlag
                    );

                    CHECKCANCEL(_doStreamDepth);
                    if (recomputeSucceed.errorCode == B3DNativeProcessError::B3D_RECALIBRATION_CALIB_NO_ERROR ) {
                        logInfo("Auto Recalibration succeed");
                        // export data to _calibrationdir
                        // for auto recalibration, only save files to as tmp file
                        const std::string tmpRightCamFile = "rightCam_tmp.yml";
                        const std::string tmpCalibBinFile = "b3dCalibData_tmp.bin";
                        const std::string tmpCalibDataFile = "calib_tmp.data";
                        newCam.writeParams(_calibrationdir + tmpRightCamFile, false);
                        writeB3DCalibrationData(_calibrationdir, tmpCalibBinFile, newCalibBin);
                        CameraCalibExtPtr cameraCalibExtPtr = std::make_shared<b3dd::CameraCalibExt>();
                        const B3DCalibrationData *calibBinPtr = &newCalibBin;
                        cameraCalibExtPtr->cacheCalibrationData(
                                (unsigned char *) calibBinPtr,
                                _calibrationdir + tmpCalibDataFile,
                                DepthCameraType::DEPTHCAM_B3D4
                        );
                        _cameraCalibExtPtr->loadFromFile(_calibrationdir + tmpCalibDataFile);
                        _depthProcessor.loadCalibrationData(_cameraCalibExtPtr);
                    } else if(recomputeSucceed.errorCode == B3DNativeProcessError::B3D_RECALIBRATION_CALIB_NO_CHANGE){
//                        //do nothing
                        logInfo("Auto Recalibration no need to change");
                    } else {
                        logError("Skip Auto Recalibration, error code %d",recomputeSucceed.errorCode);
                    }

                    if(_processListener)
                        _processListener->onRecalibrationDone(recomputeSucceed, calibDispErr);

                    logInfo("Auto Recalibration: Fac cy[%.2f] Local cy[%.2f] autoRecalib cy[%.2f]",
                            cams[1].cy(), localCamR.cy(), newCam.cy());
                }
                CHECKCANCEL(_doStreamDepth);
                if (imageScale == 1.0f)
                {
                    _depthConfigPtr->optimizeMode = OPTIMIZE_FULL_RES;
                }
                else if (imageScale == 0.75f)
                {
                    _depthConfigPtr->optimizeMode = OPTIMIZE_HALF_RES;
                }
                else if (imageScale == 0.5f)
                {
                    _depthConfigPtr->optimizeMode = OPTIMIZE_QUARTER_RES;
                }

                _depthConfigPtr->depthScale = imageScale;

                if (framecounter == 0) // first depth frame uses IRThres
                    _depthConfigPtr->useIRThres = true;
                else
                    _depthConfigPtr->useIRThres = false;

                _depthProcessor.updateProcessorSettingsExt(_depthConfigPtr);

                logVerbose("Input image size (%d, %d) (%d, %d), scale %.2f",
                        front.first.cols(), front.first.rows(), front.second.cols(), front.second.rows(),
                        imageScale);
                CHECKCANCEL(_doStreamDepth);
                computetesult = _depthProcessor.computeDepth(front.first, front.second, outImage);
                CHECKCANCEL(_doStreamDepth);
                logVerbose("Output depth size (%d, %d)",outImage.cols(), outImage.rows());

                logInfo("computeDepth (state, count, store ) -> (%d, %d, %d)", computetesult, framecounter);
                if (computetesult) {
                    B3DImage showDepth;
                    cv::Mat depthImage;
                    B3DImageToCvMat(outImage, depthImage);

                    logVerbose("depthFrame size (%d, %d)", depthImage.cols, depthImage.rows);

#ifdef ENCODEPNG
                    const double startPngEncode = b3di::now_ms();
                    pngParam[0] = CV_IMWRITE_PNG_COMPRESSION;
                    pngParam[1] = 3;
                    cv::imencode(".png",depthImage,pngBuff,pngParam);
                    logInfo("imencode png -- %.2f", b3di::now_ms() - startPngEncode);
#endif
                    if(_debugFlag)
                        saveDepth(_sessiondir, framecounter, depthImage);

                    if(_processListener)
                        _processListener->onStreamDepthDone(depthImage,timeStampL);
                    else
                        logWarning("_processListener is null");

                    framecounter++;
                    _depthProcessor.printTimingStatistics();
                } else {
                    logError("Compute depth error");
                    finishProcessing();
                }
                CHECKCANCEL(_doStreamDepth);
            } catch (b3dd::B3D_DEPTH_COMPUTATION_ERROR& err) {

                B3DNativeProcessError nativehErr;
                switch(err) {
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_CONFIG_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_CONFIG_INVALID,"Invalid depth config");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_IMAGE_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_IMAGE_INVALID,"Invalid depth image");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_ROI_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_ROI_INVALID,"Invalid depth ROI");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INTPUT_CALIBATION_DATA_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_CALIBATION_DATA_INVALID,"Invalid depth calibration data");
                        break;
                    case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INTPUT_RECT_MAP_INVALID :
                        nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_RECT_MAP_INVALID,"Invalid rect map");
                        break;
                    default:
                        break;
                }

                logError("compute depth with error %s",(nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                return;
            } catch( cv::Exception& e ) {
                B3DNativeProcessError nativehErr;
                const char* err_msg = e.what();
                nativehErr = B3DNativeProcessError(
                        B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                        err_msg);
                logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
                if(_processListener)
                    _processListener->onError(nativehErr);
                return;
            } catch (...) {
                auto expPtr = std::current_exception();
                B3DNativeProcessError nativehErr;
                try {
                    if(expPtr) std::rethrow_exception(expPtr);
                } catch(const std::exception& e) {
                    const char* err_msg = e.what();
                    nativehErr = B3DNativeProcessError(
                            B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                            err_msg);
                    logError("some error occur %s",(nativehErr.debugMessage).c_str());
                    if(_processListener)
                        _processListener->onError(nativehErr);
                    return;
                }
            }
            CHECKCANCEL(_doStreamDepth);
        }
        CHECKCANCEL(_doStreamDepth);
    }

    if(_processListener)
        _processListener->onProcessFinished(__func__);

    logInfo("doStreamDepth -- %.2f", b3di::now_ms() - startStreamDepthWork);
    logVerbose("X");
}

void CameraStreamProcessorImpl::doSingleDepthComputation(FrameImage IRL, FrameImage IRR, FrameImage &depth, const float imageScale) {
    logVerbose("E");

    const double doSingleDepthComputation = b3di::now_ms();

    try {
        logVerbose("start");
        B3DImage ImageL, ImageR, outImage;
        bool computetesult = false;
        CvMatToB3DImage(IRL, ImageL, imageScale);
        CvMatToB3DImage(IRR, ImageR, imageScale);

        // actually, should not call like this way...
        initProcessParameters("/sdcard/Bellus3d/Arc/ArcClient/","",
                              1,false,false);
        if (imageScale == 1.0f)
        {
            _depthConfigPtr->optimizeMode = OPTIMIZE_FULL_RES;
        }
        else if (imageScale == 0.75f)
        {
            _depthConfigPtr->optimizeMode = OPTIMIZE_HALF_RES;
        }
        else if (imageScale == 0.5f)
        {
            _depthConfigPtr->optimizeMode = OPTIMIZE_QUARTER_RES;
        }

        _depthConfigPtr->depthScale = imageScale;
        _depthConfigPtr->useIRThres = false;

        _depthProcessor.updateProcessorSettingsExt(_depthConfigPtr);
        computetesult = _depthProcessor.computeDepth(ImageL, ImageR, outImage);
        logVerbose("Output depth size (%d, %d)",outImage.cols(), outImage.rows());

        if (computetesult) {
            B3DImage showDepth;
            cv::Mat depthImage;
            B3DImageToCvMat(outImage, depthImage);

            const float targetDepthImageScale = 0.75f;
            if (imageScale != targetDepthImageScale) {
                cv::resize(depthImage, depthImage, cv::Size(),
                           targetDepthImageScale / imageScale, targetDepthImageScale / imageScale, cv::INTER_NEAREST);
            }

            logVerbose("depthFrame size (%d, %d)", depthImage.cols, depthImage.rows);
            depth = depthImage;
            _depthProcessor.printTimingStatistics();
        } else {
            logError("Compute depth error");
            depth = cv::Mat();
        }
    } catch (b3dd::B3D_DEPTH_COMPUTATION_ERROR& err) {

        B3DNativeProcessError nativehErr;
        switch(err) {
            case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_CONFIG_INVALID :
                nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_CONFIG_INVALID,"Invalid depth config");
                break;
            case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_IMAGE_INVALID :
                nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_IMAGE_INVALID,"Invalid depth image");
                break;
            case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INPUT_ROI_INVALID :
                nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_ROI_INVALID,"Invalid depth ROI");
                break;
            case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INTPUT_CALIBATION_DATA_INVALID :
                nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_CALIBATION_DATA_INVALID,"Invalid depth calibration data");
                break;
            case b3dd::B3D_DEPTH_COMPUTATION_ERROR::INTPUT_RECT_MAP_INVALID :
                nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_RECT_MAP_INVALID,"Invalid rect map");
                break;
            default:
                break;
        }

        logError("compute depth with error %s",(nativehErr.debugMessage).c_str());
        //TODO find a way to return error code to java
//        if(_processListener)
//            _processListener->onError(nativehErr);
        depth = cv::Mat();
        return;
    } catch( cv::Exception& e ) {
        B3DNativeProcessError nativehErr;
        const char* err_msg = e.what();
        nativehErr = B3DNativeProcessError(
                B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                err_msg);
        logError("compute depth with error %s", (nativehErr.debugMessage).c_str());
//        if(_processListener)
//            _processListener->onError(nativehErr);
        depth = cv::Mat();
        return;
    } catch (...) {
        auto expPtr = std::current_exception();
        B3DNativeProcessError nativehErr;
        try {
            if(expPtr) std::rethrow_exception(expPtr);
        } catch(const std::exception& e) {
            const char* err_msg = e.what();
            nativehErr = B3DNativeProcessError(
                    B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR,
                    err_msg);
            logError("some error occur %s",(nativehErr.debugMessage).c_str());
//            if(_processListener)
//                _processListener->onError(nativehErr);
            depth = cv::Mat();
            return;
        }
    }

    logInfo("doSingleDepthComputation -- %.2f", b3di::now_ms() - doSingleDepthComputation);
    logVerbose("X");

}

void CameraStreamProcessorImpl::saveFrame() {
    logVerbose("E");
    if(_colorFrameStore.empty() || _stereoComposer.empty() || _depthFrameStore.empty()) {
        logWarning("Can't save all debug Frames, some container is empty C %d, S %d, D %d",_colorFrameStore.empty(),_stereoComposer.empty(),_depthFrameStore.empty());
    }

    char value[255];
    int debugSaveColor = 0;
#ifdef __ANDROID__
    __system_property_get("debug.bellus3d.b3d4client.savecolor",value);
    debugSaveColor = atoi(value);
#endif

    for(int x = 0 ; x < _stereoComposer.size() ; x ++) {
        if(debugSaveColor == 1)
            cv::imwrite(_sessiondir + "/Results/M/M_" + std::to_string(x) + ".png",_colorFrameStore[x]);
        std::string IRLName = "/Results/L/L_" + std::to_string(x) + "_" + std::to_string((!_timeStampL.empty() &&_timeStampL[x] > 0) ?
                _timeStampL[x] : 00000) + ".png";
        std::string IRRName = "/Results/R/R_" + std::to_string(x) + "_" + std::to_string((!_timeStampR.empty() &&_timeStampR[x] > 0) ?
                _timeStampR[x] : 00000) + ".png";
        cv::imwrite(_sessiondir + IRLName, _stereoComposer[x].first);
        cv::imwrite(_sessiondir + IRRName, _stereoComposer[x].second);
    }
    for(int x = 0 ; x < _depthFrameStore.size() ; x ++) {
        if (!_depthFrameStore[x].empty())
            cv::imwrite(_sessiondir + "/Results/D/D_" + std::to_string(x) + ".png", _depthFrameStore[x]);
    }
    logVerbose("X");
}

void CameraStreamProcessorImpl::resetPrivateParameters() {
    logVerbose("E");
    _stereoFrameStore = std::queue<std::pair<B3DImage, B3DImage>>();
    _stereoFrameScale = std::queue<float>();
    _stereoLFrameTimeStamp = std::queue<long>();
    _doDepth = false;
    _doFaceDetect = false;
    _doKeyFrameSelect = false;
    _doReCalibration = false;
    _doGenFaceLandMark = false;
    _doStreamDepth = false;
    _reCalibrationReady = true;
    frameEnoughReported = false;
    _depthFrameStore.clear();
    _colorFrameStore.clear();
    _timeStampM.clear();
    _timeStampL.clear();
    _timeStampR.clear();
    _stereoComposerStatus.clear();
    _stereoComposer.clear();
    _stereoFramesCounter = 0;
    _isProcessing = false;
    _isFaceDetectProcessing = false;
    _isGenFaceLandMarkProcessing = false;
    _isRecalibrationProcessing = false;
    _isStreamDepthProcessing = false;
    _debugFlag = false;
    logVerbose("X");
}
void CameraStreamProcessorImpl::resetControlFlags() {
    logVerbose("E");
    _doDepth = false;
    _doFaceDetect = false;
    _doKeyFrameSelect = false;
    _doReCalibration = false;
    _doGenFaceLandMark = false;
    _doStreamDepth = false;
    _reCalibrationReady = true;
    frameEnoughReported = false;
    _stereoFramesCounter = 0;
    _isProcessing = false;
    _isFaceDetectProcessing = false;
    _isGenFaceLandMarkProcessing = false;
    _isRecalibrationProcessing = false;
    _isStreamDepthProcessing = false;
    _debugFlag = false;
    logVerbose("X");
}
}  // End of namespace b3d4
