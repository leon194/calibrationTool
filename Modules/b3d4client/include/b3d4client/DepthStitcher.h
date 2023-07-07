#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <thread>


#include <b3d4client/B3DCameraError.h>
#include <b3d4client/B3DNativeProcessError.h>
#include <b3d4client/B3DCameraStreamListener.h>

#include <b3di/mergedepth.h>  // <- this is better hidden in implementation

namespace b3d4 {

class DLLEXPORT B3DStitcher {

public:
    B3DStitcher();
    virtual ~B3DStitcher();

    /* Stitch single view frames together */
    bool stitchDepth(const b3di::MergeDepth_Input& input, b3di::MergeDepth_Output& output);

    /* update stitcher setting, should we write a stitchersetting.cpp? */
    B3DNativeProcessError initStitcherSetting(std::string rootPath, std::string sessionPath, int keyFrameNum);

    void initStitcherThr();

    void startStitcher();

    void doStitcherWork();

    bool isStitching() { return _doSticher; };

    void setStitcherInputHeadRect(cv::Rect &faceRect);

    void setStitcherInputColor(std::vector<cv::Mat> &colorFrame);

    void setStitcherInputDepth(std::vector<cv::Mat> &depthFrame);

    void registerProcessListener(B3DCameraProcessListenerPtr processListenerPtr);

    void initStitcherInputHeadMask(const float faceRect[], const float faceDistance[], const trimesh::xform &xf);

    void updateHeadMask();

    void resetPrivateParameters();

    void finishStitcher();

    void cancelStitcher();

    b3di::CameraParams depthCam, rgbCam;
    b3di::MergeDepth_Input input;

private:
    std::string _rootdir;
    std::string _sessiondir;
    std::string _facDataDir;

    /* frames container */
    std::vector<cv::Mat> _depthFrame;
    std::vector<cv::Mat> _colorFrame;

    std::thread _doSticherThr;
    bool _doSticher, _startStitcher;
    B3DNativeProcessError _nativehErr;

    float *_faceRect;
    float _faceDistance;
    trimesh::xform _xf;

    B3DCameraProcessListenerPtr _processListener;


};  // End of B3DStitcher class

using B3DStitcherPtr = std::shared_ptr<B3DStitcher>;

}  // End of namespace b3d4
