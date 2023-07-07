#include <sys/stat.h>
#include <b3d4client/B3DConfig.h>
#include <b3ddepth/utils/TLog.h>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <b3di/B3d_utils.h>
#include <fstream>
#include <string>

#include "DepthStitcher.h"

#ifdef __ANDROID__
#include "backTrace.h"
#endif

using namespace cv;

namespace b3d4 {

#define CHECKCANCEL(flag) if (!flag) {                  \
    logInfo("finishing %s",__func__);                   \
    if(_processListener)                                \
        _processListener->onProcessFinished(__func__);  \
    else logWarning("_processListener is null");        \
    return;                                             \
}

    bool CheckExists(const std::string& name) {
        struct stat buffer;
        return ( stat (name.c_str(), &buffer) == 0 );
    }

    B3DStitcher::B3DStitcher()
    {
        logVerbose("E");
        resetPrivateParameters();
        _processListener = nullptr;
        _nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_NO_ERROR,"No Error!");
#ifdef __ANDROID__
        signal(SIGSEGV, signalHandler);
#endif
        logVerbose("X");
    }

    B3DStitcher::~B3DStitcher()
    {
        logVerbose("E");
        _processListener = nullptr;
//        if(_faceRect != nullptr) {
//            delete _faceRect;
//            _faceRect = nullptr;
//        }
        logVerbose("X");
    }

    static void computeHeadMask(
            const cv::Rect &inputFaceRect,
            float inputFaceDistance,
            const b3di::CameraParams &faceRectCam,
            const b3di::CameraParams &depthCam,
            const trimesh::xform &xf,
            cv::Mat &headMask,
            float topYOffset, float botYOffset, float zOffset = 300.0f)
    {
        logVerbose("E");
        const cv::Size inputSize = faceRectCam.getImageSize();
        const cv::Rect faceRectBoundary(0, 0, inputSize.width, inputSize.height);

        // compute 3d pts
        float faceDistanceOffset = 30.0f;

        const float faceDistance = inputFaceDistance - faceDistanceOffset;
        const float headSizeZ = zOffset;
        const float fx = static_cast<float>(faceRectCam.fx());
        const float fy = static_cast<float>(faceRectCam.fy());
        const float cx = static_cast<float>(faceRectCam.cx());
        const float cy = static_cast<float>(faceRectCam.cy());

        cv::Rect faceRect = inputFaceRect;
        const int faceRectTopYAdjust =
                static_cast<int>(std::round(faceRect.height * topYOffset));
        const int faceRectBotYAdjust =
                static_cast<int>(std::round(faceRect.height * botYOffset));
        faceRect.y -= faceRectTopYAdjust;
        faceRect.height += faceRectTopYAdjust + faceRectBotYAdjust;
        b3di::fitRectToBounds(faceRectBoundary, faceRect);

        const cv::Point3f pt_01(
                (faceRect.x - cx) / fx * faceDistance,
                (faceRect.y - cy) / fy * faceDistance,
                faceDistance
        );
        const cv::Point3f pt_02(
                (faceRect.x + faceRect.width - cx) / fx * faceDistance,
                (faceRect.y - cy) / fy * faceDistance,
                faceDistance
        );
        const cv::Point3f pt_03(
                (faceRect.x - cx) / fx * faceDistance,
                (faceRect.y + faceRect.height - cy) / fy * faceDistance,
                faceDistance
        );
        const cv::Point3f pt_04(
                (faceRect.x + faceRect.width - cx) / fx * faceDistance,
                (faceRect.y + faceRect.height - cy) / fy * faceDistance,
                faceDistance
        );
        const cv::Point3f headSizeOffset(0.0f, 0.0f, headSizeZ);

        std::vector<cv::Point3f> pt3Ds{
                pt_01, pt_02, pt_03, pt_04,
                pt_01 + headSizeOffset,
                pt_02 + headSizeOffset,
                pt_03 + headSizeOffset,
                pt_04 + headSizeOffset,
        };

        // crop region
        const float shiftRatio = 0.35f;
        const cv::Point3f shift_01 = (pt3Ds[6] - pt3Ds[4]) * shiftRatio;
        const cv::Point3f shift_02 = (pt3Ds[7] - pt3Ds[5]) * shiftRatio;
        pt3Ds[4] -= shift_01;
        pt3Ds[6] -= shift_01;
        pt3Ds[5] -= shift_02;
        pt3Ds[7] -= shift_02;

        b3di::CameraParams cam = depthCam;
        cam.setCameraToWorldXform(xf);
        std::vector<cv::Point2f> pts;
        b3di::xformAndProjectPoints(pt3Ds, faceRectCam, cam, pts);

        const cv::Size imageSize = cam.getImageSize();
        cv::Mat mask = cv::Mat::zeros(imageSize, CV_8UC1);
        const std::vector<std::vector<cv::Point>> contours{
                { pts[0], pts[1], pts[3], pts[2] },
                { pts[4], pts[5], pts[7], pts[6] },
                { pts[0], pts[1], pts[5], pts[4] },
                { pts[1], pts[3], pts[7], pts[5] },
                { pts[3], pts[2], pts[6], pts[7] },
                { pts[2], pts[0], pts[4], pts[6] },
        };
        for (int n = 0; n != contours.size(); ++n)
            cv::drawContours(mask, contours, n, cv::Scalar(255), -1, 8);

        headMask = mask;
        logVerbose("X");
        return;
    }

    B3DNativeProcessError B3DStitcher::initStitcherSetting(std::string rootPath, std::string sessionPath,
            int keyFrameNum) {
        logVerbose("E");
        _rootdir = rootPath;
        _sessiondir = sessionPath;
        _facDataDir = "/mnt/vendor/";

        logVerbose("rootPath %s, sessiondir %s, faceDataDir %s",_rootdir.c_str(), _sessiondir.c_str(), _facDataDir.c_str());

        /* check if CalibrationFiles folder exist */
        if(!CheckExists(_rootdir + "CalibrationFiles")) {
            logWarning("CalibrationFiles does not exist, copy from factory partition !");
            if(!CheckExists(_facDataDir + "CalibrationFiles")) {
                std::string errMessage = "CalibrationFiles does not exist in factory partition !";
                logError("%s",errMessage.c_str());
                _nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_SINGLE_VIEW_MERGE_INPUT_CALIBRATION_DATA_INVALID, \
                                        errMessage);
                return _nativehErr;
            }
            /* copy folder from factory partition */
            std::system("cp /mnt/vendor/CalibrationFiles/ -R /sdcard/Bellus3d/Arc/ArcClient/");
            /* copy leftCam.yml to depthCam.yml*/
            std::system("cp /sdcard/Bellus3d/Arc/ArcClient/CalibrationFiles/leftCam.yml /sdcard/Bellus3d/Arc/ArcClient/CalibrationFiles/depthCam.yml ");
        } else if(!CheckExists(_rootdir + "CalibrationFiles/depthCam.yml")) {
            std::system("cp /sdcard/Bellus3d/Arc/ArcClient/CalibrationFiles/leftCam.yml /sdcard/Bellus3d/Arc/ArcClient/CalibrationFiles/depthCam.yml ");
        }

        /* config sticher */

        depthCam.loadParams(_rootdir + "CalibrationFiles/depthCam.yml");

        if (depthCam.empty()) {
            logError("Can't load file depthCam");
        }

        rgbCam.loadParams(_rootdir + "CalibrationFiles/midCam.yml");

        if (rgbCam.empty()) {
            logError("Can't load file midCam");
        }

        if (depthCam.empty() || rgbCam.empty()) {
            std::string errMessage = "Merge depthCam.yml or midCam.yml empty  !";
            logError("%s",errMessage.c_str());
            _nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_SINGLE_VIEW_MERGE_INPUT_CALIBRATION_DATA_INVALID, \
                                        errMessage);
            if(_processListener)
                _processListener->onError(_nativehErr);
            return _nativehErr;
        }
        
        // set input
        input.depthCam = depthCam;
        input.keyFrameIdx = keyFrameNum;
        input.depthCamType = b3di::DepthCamType::DEPTHCAM_B3D4;
        input.minSampleNum = 2;
        input.headMask = cv::Mat();
        logVerbose("X");
        return _nativehErr;
    };

    void B3DStitcher::initStitcherThr() {
        logVerbose("E");
        if(!_doSticher) {
            _doSticherThr = std::thread(&B3DStitcher::doStitcherWork, this);
            _doSticherThr.detach();
            _doSticher = true;
        }
        logVerbose("X");
    };

    void B3DStitcher::startStitcher() {
        logVerbose("E");
        _startStitcher = true;
        logVerbose("X");
    };

    void B3DStitcher::setStitcherInputHeadRect(cv::Rect &faceRect) {
        logVerbose("E");
#if 0
#ifdef DOFACESELECT
        input.headRect = faceRect;
#else
        // !!! hard code face Rect before we have head tracker
        const cv::Size depthSize = input.inputDepthImages.front().size();
        const int faceRectRatio = 5;
        const int x = depthSize.width / faceRectRatio;
        const int y = depthSize.height / faceRectRatio;
        const int rectWidth = depthSize.width - x;
        const int rectHeight = depthSize.height - y * 2;
        input.headRect = Rect(x, y, rectWidth, rectHeight);
#endif
#endif
        logVerbose("X");
    };

    void B3DStitcher::setStitcherInputColor(std::vector<cv::Mat> &colorFrame) {
        logVerbose("E");
        logVerbose("X");
    };

    void B3DStitcher::setStitcherInputDepth(std::vector<cv::Mat> &depthFrame) {
        logVerbose("E");
        input.inputDepthImages = depthFrame;
        logVerbose("inputDepthImages %d",input.inputDepthImages.size());
        logVerbose("X");
    };

    void B3DStitcher::initStitcherInputHeadMask(const float pfaceRect[], const float pfaceDistance[], const trimesh::xform &pxf) {
        logVerbose("E");
        logVerbose("pfaceRect %f, %f, %f, %f",pfaceRect[0],pfaceRect[1],pfaceRect[2],pfaceRect[3]);
        for(int x = 0 ; x < 4 ; x++)
            _faceRect[x] = pfaceRect[x];
        _faceDistance = pfaceDistance[2];
        _xf = pxf;
        logVerbose("_faceRect %f, %f, %f, %f, _faceDistance %d",_faceRect[0],_faceRect[1],_faceRect[2],_faceRect[3],_faceDistance);
        logVerbose("X");
    };

    void B3DStitcher::updateHeadMask() {
        logVerbose("E");
        const int width = rgbCam.getImageSize().width;
        const int height = rgbCam.getImageSize().height;

        std::vector<float> defaultFaceRect{0.25f, 0.20f, 0.5f, 0.6f};
        cv::Rect faceRect;
        if (_faceRect[0] <= 0 || _faceRect[1] <= 0 || _faceRect[2] <= 0 || _faceRect[3] <= 0
            || _faceRect[0] >= 1 || _faceRect[1] >= 1 || _faceRect[2] >= 1 || _faceRect[3] >= 1)
        {
            faceRect = cv::Rect(
                    static_cast<int>(std::round(width * defaultFaceRect[0])),
                    static_cast<int>(std::round(height * defaultFaceRect[1])),
                    static_cast<int>(std::round(width * defaultFaceRect[2])),
                    static_cast<int>(std::round(height * defaultFaceRect[3]))
            );
        } else{
            faceRect = cv::Rect(
                    static_cast<int>(std::round(width * _faceRect[0])),
                    static_cast<int>(std::round(height * _faceRect[1])),
                    static_cast<int>(std::round(width * _faceRect[2])),
                    static_cast<int>(std::round(height * _faceRect[3]))
            );
        }

        logInfo("faceRect %d, %d, %d, %d",faceRect.x,faceRect.y,faceRect.width,faceRect.height);
        logInfo("faceDistance %f",_faceDistance);

        cv::Mat headMask;
        if (_faceDistance > 300 && _faceDistance < 500)
        {
            computeHeadMask(faceRect, _faceDistance, rgbCam, depthCam, _xf,
                            headMask, 0, 0);
        }

        input.headMask = headMask;
        logVerbose("X");
    }

    void B3DStitcher::doStitcherWork() {
        b3dd::TLog::setLogFile((_sessiondir + ("/processing.log")).c_str(), false);
        logVerbose("E");
        while (_doSticher) {
            if (_startStitcher) {
                logInfo("start merge");
                logVerbose("inputDepthImages %d",input.inputDepthImages.size());
                logVerbose("depthCam %d",input.depthCam.empty());
                logVerbose("headMask %d",input.headMask.empty());
                logVerbose("_faceRect x : %d, y : %d, w : %d, h : %d",_faceRect[0],_faceRect[1],_faceRect[2],_faceRect[3]);
                logVerbose("_faceDistance %d",_faceDistance);
                CHECKCANCEL(_doSticher);
                try {
                    std::fstream  file ;
                    file.open(_sessiondir+"/rect_and_distance.txt", std::ios::out | std::ios::trunc);
                    std::string data =  "faceRect x : " + std::to_string(_faceRect[0]) + "\n";
                    file << data;
                    data =  "faceRect y : " + std::to_string(_faceRect[1])+ "\n";
                    file << data;
                    data =  "faceRect w : " + std::to_string(_faceRect[2])+ "\n";
                    file << data;
                    data =  "faceRect h : " + std::to_string(_faceRect[3])+ "\n";
                    file << data;
                    data =  "faceDistance : " + std::to_string(_faceDistance)+ "\n";
                    file << data;
                    file.close();
                    _xf.write(_sessiondir + "/xf.txt");
                    const double startMerge = b3di::now_ms();
                    bool stitchResult = false;
                    CHECKCANCEL(_doSticher);
                    // add this to avoid output get released during cancel
                    b3di::MergeDepth_Output output;
                    stitchResult = stitchDepth(input, output);
                    CHECKCANCEL(_doSticher);
                    if (stitchResult) {
                        logInfo("merge success -- %.2f", b3di::now_ms() - startMerge);
                        if(_processListener)
                            _processListener->onStitcherDone(
                                output.depthImage,
                                output.confidenceMap,
                                output.rmseMap,
                                output.irMask
                        );
                        _startStitcher = false;
                        _doSticher = false;
                    }
                    else {
                        std::string errMessage = "merge failed";
                        _nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_SINGLE_VIEW_MERGE_FAILED, \
                                        errMessage);
                        if(_processListener)
                            _processListener->onError(_nativehErr);
                        logError("%s",errMessage.c_str());
                        _startStitcher = false;
                        _doSticher = false;
                        return;
                    }
                } catch( cv::Exception& e ) {
                    B3DNativeProcessError nativehErr;
                    const char* err_msg = e.what();
                    nativehErr = B3DNativeProcessError(
                            B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR,
                            err_msg);
                    logError("stitchDepth with error %s", (nativehErr.debugMessage).c_str());
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
                        logError("stitchDepth some error occur %s",
                                 (nativehErr.debugMessage).c_str());
                        if(_processListener)
                            _processListener->onError(nativehErr);
                        return;
                    }
                }
            } else {
                logVerbose("wait for start Stitcher flag %d",_startStitcher);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            CHECKCANCEL(_doSticher);
        }

        /* if without cancel processing, report finish to java for reset curHostCommand*/
        if(_processListener)
            _processListener->onProcessFinished(__func__);
        // move reset here since we end the processing
        resetPrivateParameters();
        logVerbose("X");
    }

    void B3DStitcher::registerProcessListener(B3DCameraProcessListenerPtr processListenerPtr) {
        logVerbose("E");
        _processListener = processListenerPtr;
        logVerbose("X");
    };

    bool B3DStitcher::stitchDepth(const b3di::MergeDepth_Input& input, b3di::MergeDepth_Output& output) {
        return b3di::mergeDepth(input, output);
    };

    void B3DStitcher::resetPrivateParameters() {
        logVerbose("E");
        _doSticher = false;
        _startStitcher = false;
        _rootdir = "";
        _sessiondir = "" ;
        _facDataDir = "";
        _depthFrame.clear();
        _colorFrame.clear();
        _faceRect = new float[4];
        _faceDistance = 0.0;

        input.keyFrameIdx = 0;
        input.depthCamType = b3di::DepthCamType::DEPTHCAM_B3D4;
        input.minSampleNum = 2;
        logVerbose("X");
    };

    void B3DStitcher::finishStitcher() {
        logVerbose("E");
        _doSticher = false;
        _startStitcher = false;
        logVerbose("X");
    };

    void B3DStitcher::cancelStitcher() {
        logVerbose("E");
        _doSticher = false;
        _startStitcher = false;
        logVerbose("X");
    };



}
