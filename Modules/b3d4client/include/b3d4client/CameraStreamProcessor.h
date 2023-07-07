#pragma once

#include "B3DCameraStreamListener.h"

#include <b3ddepth/core/B3DVersion.h>
#include <b3ddepth/core/DepthProcessor.h>
#include <b3ddepth/core/ext/DepthProcessorExt.h>
#include <b3ddepth/core/ext/DepthConfigExt.h>

using namespace b3dd;

namespace b3d4 {

class DLLEXPORT CameraStreamProcessor {

public:
    CameraStreamProcessor();
    virtual ~CameraStreamProcessor();

    enum ProcessType {
        PROCESS_TYPE_FACE,
        PROCESS_TYPE_SINGLEVIEW,
        PROCESS_TYPE_FACELANDMARK,
        PROCESS_TYPE_RECALIBRATION,
        PROCESS_TYPE_DEPTHCOMPUTATION,
        PROCESS_TYPE_DIAGNOSTIC
    };

    static std::shared_ptr<CameraStreamProcessor> newCameraStreamProcessor();

    virtual void InitProcessingParameter() = 0;

    // override DepthCameraStreamListener::onFrame
    // called when new frames are available
    virtual void addFrame(const B3DCameraFramePtr framePtr) = 0;

    // called when new face detection frames are available
    virtual void addFrameFaceDetect(const B3DCameraFramePtr framePtr) = 0;

    // called when new FaceLandMark frames are available
    virtual void addFrameGenFaceLandMark(const B3DCameraFramePtr framePtr) = 0;

    // called when new recalibration frames are available
    virtual void addFrameRecalibration(const B3DCameraFramePtr framePtr) = 0;

    // called when new depth computation frames are available
    virtual void addFrameStreamDepth(const B3DCameraFramePtr framePtr) = 0;

    // will start a background thread to go through stereo & rgb framestores
    // compute the depth map (store in memory)
    // whether we remove raw stereo & rgb frames from memory is an option

    // if frame stores are empty, thread will check back every T=200? ms
    // until stopProcessing is called
    virtual void startProcessing() = 0;

    // will stop adding new raw frames
    // worker thread will finish once existing frames are processed
    virtual void finishProcessing() = 0;

    // Will stop processing right away?
    virtual void cancelProcessing() = 0;
    
    virtual bool isProcessing() = 0;

    virtual void registerProcessListener(B3DCameraProcessListenerPtr listener) = 0;

    virtual void initProcessParameters(std::string rootdir, std::string sessiondir, int processFrame,
                                       bool doKeyFrameSelection, bool needManualRecalib) = 0;

    virtual void updateProcessorSettings(DepthConfigExtPtr depthConfigPtr) = 0;

    virtual void saveFrame() = 0;

    virtual void setDebugFlag( bool enable ) = 0;

    virtual void startFaceDetect();

    virtual bool isFaceDetectProcessing();

    virtual void startGenFaceLandMark();

    virtual bool isFaceLandMarkProcessing();

    virtual void startStreamDepth();

    virtual bool isStreamDepthProcessing();

    virtual void startRecalibration();

    virtual bool isRecalibrationProcessing();

    virtual void doSingleDepthComputation(FrameImage IRL, FrameImage IRR, FrameImage &depth, const float imageScale);

//// return processed depth frames
//// invalid if processing is still ongoing
//B3DError getDepthFrames(FrameStorePtr depthFrameStore) const;
//B3DError getColorFrames(FrameStorePtr colorFrameStore) const;

    // DepthCameraStreamListenerPtr getDepthCameraStreamListener() const;

};  // End of B3DCameraStreamProcessor class

using CameraStreamProcessorPtr = std::shared_ptr<CameraStreamProcessor>;

}  // End of namespace b3d4
