#pragma once

#include "CameraStreamProcessor.h"
#include <stdio.h>
#include <queue>
#include <thread>
#include <map>

// "b3ddepth" module headers
#include <b3ddepth/core/B3DVersion.h>
#include <b3ddepth/core/DepthProcessor.h>
#include <b3ddepth/core/ext/DepthProcessorExt.h>
#include <b3ddepth/core/ext/DepthConfigExt.h>
#include <b3ddepth/core/ext/CameraCalibExt.h>
#include <b3ddepth/utils/ext/B3DCalibrationData.h>

#include <b3di/trackheadpose.h>

// B3D Stitcher
#include "DepthStitcher.h"

#include "B3D4FileNames.h"

#include <mutex>
#include <b3di/FaceTracker.h>

using namespace b3dd;


namespace b3d4 {

const int MAX_PROCESS_FRAME = 25;
const int MAX_COLOR_FRAME = 5;

class CameraStreamProcessorImpl : public CameraStreamProcessor {

public:
    CameraStreamProcessorImpl();
    virtual ~CameraStreamProcessorImpl();

    // reset container, control flag...etc
    virtual void InitProcessingParameter();

    // override B3DNetworkingListener::onFrame
    // called when new frames are available
    virtual void addFrame(const B3DCameraFramePtr framePtr);

    virtual void addFrameFaceDetect(const B3DCameraFramePtr framePtr);

    virtual void addFrameGenFaceLandMark(const B3DCameraFramePtr framePtr);

    virtual void addFrameRecalibration(const B3DCameraFramePtr framePtr);

    virtual void addFrameStreamDepth(const B3DCameraFramePtr framePtr);

    virtual void startProcessing();

    virtual void finishProcessing();

    virtual void cancelProcessing();

    virtual bool isProcessing();

    virtual void startFaceDetect();

    virtual bool isFaceDetectProcessing();

    virtual void startGenFaceLandMark();

    virtual bool isFaceLandMarkProcessing();

    virtual void startRecalibration();

    virtual bool isRecalibrationProcessing();

    virtual void startStreamDepth();

    virtual bool isStreamDepthProcessing();

    virtual void doSingleDepthComputation(FrameImage IRL, FrameImage IRR, FrameImage &depth, const float imageScale);

    void registerProcessListener(B3DCameraProcessListenerPtr listener);

    void initProcessParameters(std::string rootdir, std::string sessiondir, int processFrame = 15,
            bool doKeyFrameSelection = false, bool needManualRecalib = false);

    void updateProcessorSettings(DepthConfigExtPtr depthConfigPtr);

    void doFaceSelect();

    void doFaceDetectWork();

    // !!! only call it when the camera is the center one
    // select the best color image from the first 'maxFrames' of images in _colorFrameStore
    // return the index
    // pass the index to stitcher
    void doKeyFrameSelection(int maxFrames);

    // recalibrate camera params for depth computation
    // use first pair of IR images as input
    // if compute succeed, overwrite local files in _calibrationdir
    // if compute succeed, rightCam.yml, b3dCalibData.bin and calib.data will be changed
    // in local folder
    void doReCalibration();

    void doDepthWork();

    void doGenFaceLandMark();

    void doStreamDepthWork();

    // not using now
    void saveFrame();

    void resetPrivateParameters();

    void resetControlFlags();

    void setDebugFlag( bool enable ) { _debugFlag = enable; };

private:
    /* control params */
    //TODO use array to save processing control
    bool _isProcessing;
    bool _isFaceDetectProcessing;
    bool _isGenFaceLandMarkProcessing;
    bool _faceDetectReady;
    bool _isRecalibrationProcessing;
    bool _isStreamDepthProcessing;

    int _stereoFramesCounter;
    int _max_process_frame_num;
    int _max_color_frame_num;
    bool frameEnoughReported;
    std::mutex _gAddFrameLock;

    cv::Rect faceRect;

    bool _doDepth;
    std::thread _doComputeThr;

    bool _doFaceDetect;
    std::thread _doFaceDetectThr;

    bool _doGenFaceLandMark;
    std::thread _doGenFaceLandMarkThr;

    bool _doStreamDepth;
    std::thread _doStreamDepthThr;

    bool _needKeyFrameSelect;
    bool _doKeyFrameSelect;
    std::thread _doKeyFrameSelectThr;

    bool _needManualRecalib;
    bool _reCalibrationReady;
    bool _doReCalibration;
    std::thread _doReCalibrationThr;

    std::string _calibrationdir;
    std::string _rootdir;
    std::string _datadir;                    //data dir
    std::string _sessiondir;
    std::string _facDataDir;
    bool _debugFlag;

    /* listener */
    B3DCameraProcessListenerPtr _processListener;

    /* containers */
    std::queue<std::pair<B3DImage, B3DImage>> _stereoFrameStore;     // stereo frames
    std::queue<float> _stereoFrameScale;   // store corresponding image scale for stereo frames
    std::queue<long> _stereoLFrameTimeStamp;
    //std::queue<B3DCameraFramePtr> _stereoFrameStore;     // stereo frames
    //std::vector<B3DCameraFramePtr> _colorFrameStore;     // rgb frames
    std::map<int,std::pair<cv::Mat, cv::Mat>> _stereoComposer; // stereo frames <frame id , <L frame, R frame>>;
    std::map<int,int> _stereoComposerStatus; // stereo frames status <frameid, num of frameid frame receive>

    std::vector<cv::Mat> _depthFrameStore;                 // depth frames
    std::vector<cv::Mat> _colorFrameStore;                 // rgb frames
    std::vector<FrameTime> _timeStampM;
    std::vector<FrameTime> _timeStampL;
    std::vector<FrameTime> _timeStampR;

    // for head tracking
    cv::Ptr<b3di::FaceTracker> _ft;
    std::map<int, cv::Mat> _colorImages;
    std::queue<cv::Mat> _colorImageStore;

    /* depth process params */
    DepthProcessorExt _depthProcessor;
    CameraCalibExtPtr _cameraCalibExtPtr;
    DepthConfigExtPtr _depthConfigPtr;

};  // End of B3DCameraStreamProcessor class

using CameraStreamProcessorImplPtr = std::shared_ptr<CameraStreamProcessorImpl>;

}  // End of namespace b3d4
