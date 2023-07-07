#include "CameraStreamProcessor.h"

#include "CameraStreamProcessorImpl.h"

namespace b3d4 {

CameraStreamProcessor::CameraStreamProcessor() {
}

CameraStreamProcessor::~CameraStreamProcessor() {
}


CameraStreamProcessorPtr CameraStreamProcessor::newCameraStreamProcessor() {
    return CameraStreamProcessorPtr(new CameraStreamProcessorImpl());
}

void CameraStreamProcessor::InitProcessingParameter() {};

void CameraStreamProcessor::addFrame(const B3DCameraFramePtr framePtr) {};

void CameraStreamProcessor::addFrameFaceDetect(const B3DCameraFramePtr framePtr) {};

void CameraStreamProcessor::addFrameGenFaceLandMark(const B3DCameraFramePtr framePtr) {};

void CameraStreamProcessor::addFrameRecalibration(const B3DCameraFramePtr framePtr) {};

void CameraStreamProcessor::addFrameStreamDepth(const B3DCameraFramePtr framePtr) {};

void CameraStreamProcessor::startProcessing() {};

void CameraStreamProcessor::finishProcessing() {};

void CameraStreamProcessor::cancelProcessing() {};

bool CameraStreamProcessor::isProcessing() { return false; };

void CameraStreamProcessor::startFaceDetect() {};

bool CameraStreamProcessor::isFaceDetectProcessing() { return false; };

void CameraStreamProcessor::startGenFaceLandMark() {};

bool CameraStreamProcessor::isFaceLandMarkProcessing() { return false; };

void CameraStreamProcessor::startStreamDepth() {};

bool CameraStreamProcessor::isStreamDepthProcessing() { return false; };

void CameraStreamProcessor::startRecalibration() {};

bool CameraStreamProcessor::isRecalibrationProcessing() { return false; };

void CameraStreamProcessor::doSingleDepthComputation(FrameImage IRL, FrameImage IRR, FrameImage &depth, const float imageScale) {};

void registerProcessListener(B3DCameraProcessListenerPtr listener) {};

void initProcessParameters(const std::string rootdir, const std::string sessiondir, int processFrame,
        bool doKeyFrameSelection, bool needManualRecalib) {};

void updateProcessorSettings(DepthConfigExtPtr depthConfigPtr);

void saveFrame();

void setDebugFlag( bool enable );

}  // End of namespace b3d4
