#include "b3d4module.h"
#include "b3dutils/common_utils.h"
#include <b3d4client/B3DConfig.h>
#include <b3d4client/B3D4ClientJNIError.h>

#include <b3di/B3d_utils.h>
#include <b3di/FaceTracker.h>
#include <b3di/FaceTrackerASM.h>

#include <math.h>
#include <utility>
#include <sys/system_properties.h>

enum saveFormate {
    JPEG,
    PNG
};

void saveBuffer(const std::string &path, const cv::Mat buffer){
    cv::imwrite(path,buffer);
}

void saveImage(const std::string &path, const cv::Mat &image, saveFormate type){
    std::string suffix = (type == PNG) ? ".png" : ".jpg";

    const std::string outputPath
            = path + std::to_string(1) + suffix;

    cv::imwrite(outputPath, image);
}

void saveBuffers(const std::string path, std::vector<cv::Mat> &buffer,const saveFormate type){
    std::string suffix = (type == PNG) ? ".png" : ".jpg";

    for (int i = 0; i != buffer.size(); ++i) {
        const std::string outputDepth
                = path + std::to_string(i + 1) + suffix;

        cv::imwrite(outputDepth, buffer[i]);
    }
}

static void generatePreviewLandMark(cv::Mat searchImg) {
    double startFindLandMark = b3di::now_ms();

    // Detect M cam first image face landmarks
    cv::Ptr<b3di::FaceTracker> _ft = b3di::FaceTracker::newFaceTrackerPtr(b3di::FaceTracker::FT_ASM);

    std::string stasmPath = ARC_ROOT_FOLDER_PATH + "stasm4.1.0/data/";
    if (!b3di::FaceTrackerASM::initTracker(stasmPath)) {
        logError("initTracker failed, path %s",stasmPath.c_str());
        return;
    }

    int marginX = (int) (searchImg.cols * 0.1);
    int marginY = (int) (searchImg.rows * 0.1);

    cv::Rect searchRect(marginX, marginY, searchImg.cols - marginX * 2,
                        searchImg.rows - marginY * 2);
    bool foundFace = _ft->findLandmarks(searchImg, 0, false, searchRect);

    if (!foundFace) {
        logError("cannot find face in M image size=%d %d",
                 searchImg.cols, searchImg.rows);
        return;
    } else {
        std::string stasmTarget = ARC1_LANDMARK_FOLDER_PATH + "/facelandmark.yml";
        logInfo("TrackHeadPose -- %.2f", b3di::now_ms() - startFindLandMark);
        _ft->writeFile( stasmTarget, _ft->getFaceLandmarks());
    }
}

class PreviewStreamListener : public B3DCameraStreamListener {

public:

    PreviewStreamListener() {
        _startAddFrames = false;
        _startAddFaceDetectFrames = false;
        _startAddGenFaceLandMarkFrames = false;
        _startAddRecalibrationFrames = false;
        _startAddStreamDepthFrames = false;
        _startAddDiagnosticFrames = false;
    }


    void registerStreamProcessor(CameraStreamProcessorPtr b3dStreamProcessorPtr) {
        _b3dStreamProcessorPtr = std::move(b3dStreamProcessorPtr);
    };

    // Start adding frames to Stream Processor
    void startAddProcessingFrames() {
        _startAddFrames = true;
    }

    void stopAddProcessingFrames() {
        _startAddFrames = false;
    }

    void startAddFaceDetectFrames() {
        _startAddFaceDetectFrames = true;
    }

    void stopAddFaceDetectFrames() {
        _startAddFaceDetectFrames = false;
    }

    void startAddGenFaceLandMarkFrames() {
        _startAddGenFaceLandMarkFrames = true;
    }

    void stopAddGenFaceLandMarkFrames() {
        _startAddGenFaceLandMarkFrames = false;
    }

    void startAddRecalibrationFrames() {
        _startAddRecalibrationFrames = true;
    }

    void stopAddRecalibrationFrames() {
        _startAddRecalibrationFrames = false;
    }

    void startAddStreamDepthFrames() {
        _startAddStreamDepthFrames = true;
    }

    void stopAddStreamDepthFrames() {
        _startAddStreamDepthFrames = false;
    }

    void startAddDiagnosticFrames() {
        _startAddDiagnosticFrames = true;
    }

    void stopAddSDiagnosticFrames() {
        _startAddDiagnosticFrames = false;
    }

    void onFrame(B3DCameraFramePtr frameData) override {
        //  ----  Pass frame to global frame buffer, for Preview
        int frameTypeIdx = static_cast<int>(frameData->frameType);

        bool isAttached = false;
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);  // will AttachCurrent if necessary

        // Send received frames for preview
        if (g_surfaceJavaL != nullptr && frameData->frameType == B3DCameraFrame::L_FRAME) {
            renderSurfaceTexture(env, g_surfaceJavaL, frameData->frameImage);
        }
        else if (g_surfaceJavaR != nullptr &&  frameData->frameType == B3DCameraFrame::R_FRAME) {
            renderSurfaceTexture(env, g_surfaceJavaR, frameData->frameImage);
        }
        else if (g_surfaceJavaM != nullptr &&  frameData->frameType == B3DCameraFrame::M_FRAME) {
            //renderSurfaceTexture(env, g_surfaceJavaM, frameData->frameImage);
        }

        if(frameData->frameInfo.frameType == FrameInfo::FrameType::CAPTURE) {

//            long curTime = b3di::now_ms();
//            long lastTime = 0L;
//#ifdef __ANDROID__
//            char value[255] = "";
//            __system_property_get("debug.cptool.open",value);
//            lastTime = stol(value);
//#endif

            string flValue;
            ifstream fl("/sys/flood/brightness");
            fl >> flValue;
            fl.close();

            string prValue;
            ifstream pr("/sys/projector/brightness");
            pr >> prValue;
            pr.close();
            //logError("leon %s %s",flValue.c_str(), prValue.c_str());


            stringstream ss;
            std::string path = CALIBRATION_FOLDER_PATH;

            ss << setw(3) << setfill('0') << to_string(frameData->frameInfo.frameId);
            std::string frameindex = ss.str() + "_";
            std::stringstream().swap(ss);

            ss << setw(6) << setfill('0') << to_string(frameData->frameInfo.frameTime);
            std::string timeStamp = ss.str() + "_";
            std::stringstream().swap(ss);

            ss << setw(5) << setfill('0') << to_string(frameData->frameInfo.frameId);
            std::string frameindex_2 = ss.str();

//            string name = to_string(curTime - lastTime);
            string name = "";
            if(stoi(flValue) != 0)
                    name += "_flood";
            if(stoi(prValue) != 0)
                    name += "_prj";

            switch(frameData->frameType) {
                case B3DCameraFrame::L_FRAME :
                    // the capture results for calibration is preferred to have fix file name
                    // the time stamp naming should be use for continuous capture
                    // path = path + "L/L_" + frameindex + timeStamp + frameindex_2 + ".png";
                    path = path + "L/L" + name + ".png";
                    break;
                case B3DCameraFrame::R_FRAME :
                    // path = path + "R/R_" + frameindex + timeStamp + frameindex_2 + ".png";
                    path = path + "R/R" + name + ".png";
                    break;
                case B3DCameraFrame::M_FRAME :
                    // path = path + "M/M_" + frameindex + timeStamp + frameindex_2 + ".png";
                    path = path + "M/M" + name + ".png";
                    break;
                default:
                    path="";
                    break;
            }

            cv::imwrite(path,frameData->frameImage);
        }

        if (isAttached) {
            g_jvm->DetachCurrentThread();
        }

        if (_startAddFrames) {
            _b3dStreamProcessorPtr->addFrame(frameData);
        } else if (_startAddFaceDetectFrames || _startAddDiagnosticFrames) {
            _b3dStreamProcessorPtr->addFrameFaceDetect(frameData);
        } else if (_startAddGenFaceLandMarkFrames) {
            _b3dStreamProcessorPtr->addFrameGenFaceLandMark(frameData);
        } else if (_startAddRecalibrationFrames) {
            _b3dStreamProcessorPtr->addFrameRecalibration(frameData);
        } else if (_startAddStreamDepthFrames) {
            _b3dStreamProcessorPtr->addFrameStreamDepth(frameData);
        }

    }


private:
    bool _startAddFrames;
    bool _startAddFaceDetectFrames;
    bool _startAddGenFaceLandMarkFrames;
    bool _startAddRecalibrationFrames;
    bool _startAddStreamDepthFrames;
    bool _startAddDiagnosticFrames;

    CameraStreamProcessorPtr _b3dStreamProcessorPtr;
};


jint JNIErrorCode(const B3DNativeProcessError err) {

    jint errcode = static_cast<jint>(B3D_ERROR_CATEGORY::B3D_NO_ERROR);

    switch(err.errorCode) {
        case B3DNativeProcessError::ErrorCode::B3D_FILE_MISSING :
            errcode = static_cast<jint>(B3D_FILE_IO_ERROR::FILE_MISSING);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_FILE_READ_FAIL :
            errcode = static_cast<jint>(B3D_FILE_IO_ERROR::FILE_READ_FAIL);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_FILE_WRITE_FAIL :
            errcode = static_cast<jint>(B3D_FILE_IO_ERROR::FILE_WRITE_FAIL);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_FILE_FORMAT_NOT_SUPPORTED :
            errcode = static_cast<jint>(B3D_FILE_IO_ERROR::FILE_FORMAT_NOT_SUPPORTED);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_CONFIG_INVALID :
            errcode = static_cast<jint>(B3D_DEPTH_COMPUTATION_ERROR::INPUT_CONFIG_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_RECT_MAP_INVALID :
            errcode = static_cast<jint>(B3D_DEPTH_COMPUTATION_ERROR::INTPUT_RECT_MAP_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_DEPTH_INTPUT_CALIBATION_DATA_INVALID :
            errcode = static_cast<jint>(B3D_DEPTH_COMPUTATION_ERROR::INTPUT_CALIBATION_DATA_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_ROI_INVALID :
            errcode = static_cast<jint>(B3D_DEPTH_COMPUTATION_ERROR::INPUT_ROI_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_DEPTH_INPUT_IMAGE_INVALID :
            errcode = static_cast<jint>(B3D_DEPTH_COMPUTATION_ERROR::INPUT_IMAGE_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_SINGLE_VIEW_MERGE_SETTINGS_INVALID :
            errcode = static_cast<jint>(B3D_SINGLE_VIEW_MERGE_ERROR::SETTINGS_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_SINGLE_VIEW_MERGE_INPUT_DEPTH_INVALID :
            errcode = static_cast<jint>(B3D_SINGLE_VIEW_MERGE_ERROR::INPUT_DEPTH_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_SINGLE_VIEW_MERGE_INPUT_CALIBRATION_DATA_INVALID :
            errcode = static_cast<jint>(B3D_SINGLE_VIEW_MERGE_ERROR::INPUT_CALIBRATION_DATA_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_SINGLE_VIEW_MERGE_FAILED :
            errcode = static_cast<jint>(B3D_SINGLE_VIEW_MERGE_ERROR::MERGE_FAILED);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_RECALIBRATION_INPUT_IMAGE_INVALID :
            errcode = static_cast<jint>(B3D_RECALIBRATION_ERROR::INPUT_IMAGE_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_RECALIBRATION_CALIB_SIZE_INVALID :
            errcode = static_cast<jint>(B3D_RECALIBRATION_ERROR::CALIB_SIZE_INVALID);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_RECALIBRATION_CALIB_NO_CHANGE :
            errcode = static_cast<jint>(B3D_RECALIBRATION_ERROR::CALIB_NO_CHANGE);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_RECALIBRATION_CALIB_NO_ERROR :
            errcode = static_cast<jint>(B3D_RECALIBRATION_ERROR::CALIB_NO_ERROR);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_FINDLANDMARK_FACE_NOT_FOUND :
            errcode = static_cast<jint>(B3D_FINDLANDMARK_ERROR::FACE_NOT_FOUND);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_FINDLANDMARK_INITTRACKER_INVALID :
            errcode = static_cast<jint>(B3D_FINDLANDMARK_ERROR::INIT_TRACKER_ERROR);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_OPENCV_ERROR :
            errcode = static_cast<jint>(B3D_OPENCV_ERROR::OPENCV_ERROR);
            break;
        case B3DNativeProcessError::ErrorCode::B3D_OTHER_ERROR :
            errcode = static_cast<jint>(B3D_OTHER_ERROR::OTHER_ERROR);
            break;
        default:
            errcode = static_cast<jint>(B3D_ERROR_CATEGORY::B3D_NO_ERROR);
            break;
    }

    logError("Error code %d", errcode);

    return errcode;
};

class CameraStreamProcessListener : public B3DCameraProcessListener {

public:
    CameraStreamProcessListener() :
    nativehErr(B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_NO_ERROR,"No Error!"))
    ,_jclass(nullptr)
    ,_jobject(nullptr){};

    void registerCameraPtr(B3DCameraPtr b3dCameraPtr) {
        _b3dCameraPtr = std::move(b3dCameraPtr);
    };

    void registerStitcher(B3DStitcherPtr sticherPtr) {
        _sticher = std::move(sticherPtr);
    };

    void onError(B3DNativeProcessError error) override {
        bool isAttached = false;
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        jint jresult = JNIErrorCode(error);

        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onError", "(I)V");
        env->CallVoidMethod(_jobject, javaCallback, jresult);

        if(isAttached) g_jvm->DetachCurrentThread();

    };

    void onFrameEnough(std::vector<cv::Mat> &colorFrame, int keyFrameNum, int keyFrameOffset) override {
        bool isAttached = false;
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);
        logVerbose("E");
        logInfo("colorFrame size %d keyFrameNum %d keyFrameOffset %d",colorFrame.size(),keyFrameNum,keyFrameOffset);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        _b3dCameraPtr->stopStreamSync();

        nativehErr = _sticher->initStitcherSetting(ARC_ROOT_FOLDER_PATH,ARC_PROCESSING_SESSION_FOLDER,
                                                   keyFrameNum + keyFrameOffset);

        if(nativehErr.errorCode != B3DNativeProcessError::ErrorCode::B3D_NO_ERROR) {
            onError(nativehErr);
            return;
        }

        _sticher->updateHeadMask();
        //_sticher->setStitcherInputColor(colorFrame);

        /* only save key color frame */
        std::thread save(saveBuffer,ARC_PROCESSING_SESSION_FOLDER + "/" + "Color_1.jpg",colorFrame[keyFrameNum].clone());
        /* save all color frame  */
        //std::thread save(saveBuffers,ARC_PROCESSING_SESSION_FOLDER + "/" + "Color_",std::ref(colorFrame),JPEG);
        save.detach();

        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onFrameEnough", "()V");
        env->CallVoidMethod(_jobject, javaCallback);
        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");

    }

    void onProcessDone(std::vector<cv::Mat> &depthFrame, cv::Rect &faceRect) override {
        logVerbose("E");
        logVerbose("finish singleView stitch, start stitcher work");

        _b3dCameraPtr->closeSync();

        if(depthFrame.empty()) {
            std::string errMessage = "Merge inputDepthImages empty  !";
            logError("%s",errMessage.c_str());
            nativehErr = B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_SINGLE_VIEW_MERGE_INPUT_DEPTH_INVALID, \
                                        errMessage);
            onError(nativehErr);
            return;
        }

        _sticher->initStitcherThr();

        _sticher->setStitcherInputDepth(depthFrame);

        _sticher->setStitcherInputHeadRect(faceRect);

        _sticher->startStitcher();
        logVerbose("X");
    };

    void onFaceDetectDone(std::vector<float> &headPoseInfo) override {
        logVerbose("E");
        bool isAttached = false;
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        jfloatArray headPostResult  = env->NewFloatArray(static_cast<jsize>(headPoseInfo.size()));

        jfloat *data = env->GetFloatArrayElements(headPostResult, nullptr);

        for(int x = 0 ; x < headPoseInfo.size() ; x++) {
            data[x] = headPoseInfo[x];
        }

        env->SetFloatArrayRegion(headPostResult, 0, static_cast<jsize>(headPoseInfo.size()), data);


        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onFaceDetectDone", "([F)V");
        env->CallVoidMethod(_jobject, javaCallback, headPostResult);
        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");
    };

    void onGenFaceLandMarkDone(cv::Ptr<b3di::FaceTracker>& ft) override {
        logVerbose("E");
        bool isAttached = false;
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        ft->writeFile( ARC1_LANDMARK_FOLDER_PATH + "/facelandmark.yml", ft->getFaceLandmarks());

        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onGenFaceLandMarkDone", "()V");
        env->CallVoidMethod(_jobject, javaCallback);
        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");
    }

    void onRecalibrationDone(const B3DNativeProcessError error, float calibDispErr) override {
        logVerbose("E");
        bool isAttached = false;

        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        jclass jclzz = env->GetObjectClass(_jobject);

        jint jresult = JNIErrorCode(error);


        jmethodID javaCallback = env->GetMethodID(jclzz,"onRecalibrationDone", "(IF)V");
        env->CallVoidMethod(_jobject, javaCallback,jresult, (jfloat)calibDispErr);

        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");
    };

    void onStreamDepthDone(cv::Mat &depthFrame, long timeStampL) override {
        logVerbose("E");
        logInfo("StreamDepthDone , timeStampL %ld",timeStampL);

        cv::Mat toJava = depthFrame;
        bool isAttached = false;
        int bytesize = static_cast<int>(toJava.total() * toJava.elemSize());
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        jbyteArray jresult = env->NewByteArray(bytesize);

        jbyte *jbytes = env->GetByteArrayElements(jresult, nullptr);

        for ( int i = 0; i < bytesize; i++ )
        {
            jbytes[i] =  toJava.data[i];
        }

        env->SetByteArrayRegion(jresult, 0, bytesize, jbytes );

        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onStreamDepthDone", "([BJ)V");
        env->CallVoidMethod(_jobject, javaCallback, jresult, (jlong)timeStampL);
        env->ReleaseByteArrayElements(jresult,jbytes,0);
        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");
    };

    void onProcessFinished(std::string who) override {
        logVerbose("E");

        bool isAttached = false;
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        logInfo("report finishing %s",who.c_str());

        const char *cwho = who.c_str();
        jstring jstr = env->NewStringUTF(cwho);


        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onProcessFinished", "(Ljava/lang/String;)V");
        env->CallVoidMethod(_jobject, javaCallback, jstr);

        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");
    };

    void SetJavaClazz(jclass jz, jobject jo ) {
        _jclass = jz;
        _jobject = jo;
    }

private :
    B3DCameraPtr _b3dCameraPtr;
    B3DStitcherPtr _sticher;

    jclass  _jclass{};
    jobject _jobject{};

    /* Error Code */
    B3DNativeProcessError nativehErr;
};

class StitcherProcessListener : public B3DCameraProcessListener {

public :
    StitcherProcessListener() :
    nativehErr(B3DNativeProcessError(B3DNativeProcessError::ErrorCode::B3D_NO_ERROR,"No Error!"))
    ,_jobject(nullptr)
    ,_jclass(nullptr){};

    void onError(B3DNativeProcessError error) override {
        logVerbose("E");
        bool isAttached = false;
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        jint jresult = JNIErrorCode(error);

        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onError", "(I)V");
        env->CallVoidMethod(_jobject, javaCallback, jresult);

        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");
    }

    void onStitcherDone(
            cv::Mat &depthImage,
            cv::Mat &confidenceMaps,
            cv::Mat &rmseMaps,
            cv::Mat &irMask) override {
        logVerbose("E");
        logInfo("StitcherDone");

        std::thread saveDepthImage(saveImage,ARC_PROCESSING_SESSION_FOLDER + "/" + "Depth_",std::ref(depthImage),PNG);

        std::thread saveconfidenceMap(saveImage,ARC_PROCESSING_SESSION_FOLDER + "/" + "Confidence_",std::ref(confidenceMaps),PNG);

        std::thread saveRMSEImage(saveImage,ARC_PROCESSING_SESSION_FOLDER + "/" + "RMSE_",std::ref(rmseMaps),PNG);

        std::thread saveIRMask(saveImage,ARC_PROCESSING_SESSION_FOLDER + "/" + "Mask_", irMask,PNG);

        /* prepare to send frame back to Java, need to add  stitchedDepthImages and confidenceMaps further */
        cv::Mat toJava = depthImage;
        bool isAttached = false;
        int bytesize = static_cast<int>(toJava.total() * toJava.elemSize());
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        jbyteArray jresult = env->NewByteArray(bytesize);

        jbyte *jbytes = env->GetByteArrayElements(jresult, nullptr);

        for ( int i = 0; i < bytesize; i++ )
        {
            jbytes[i] =  toJava.data[i];
        }

        env->SetByteArrayRegion(jresult, 0, bytesize, jbytes );

        saveDepthImage.join();
        saveconfidenceMap.join();
        saveRMSEImage.join();
        saveIRMask.join();

        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onProcessDone", "([B)V");
        env->CallVoidMethod(_jobject, javaCallback, jresult);
        env->ReleaseByteArrayElements(jresult,jbytes,0);
        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");
    };

    void onProcessFinished(std::string who) override {
        logVerbose("E");
        bool isAttached = false;
        JNIEnv *env = getJNIEnv(g_jvm, isAttached);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return;
        }

        logInfo("report finishing %s",who.c_str());

        const char *cwho = who.c_str();
        jstring jstr = env->NewStringUTF(cwho);


        jclass jclzz = env->GetObjectClass(_jobject);
        jmethodID javaCallback = env->GetMethodID(jclzz,"onProcessFinished", "(Ljava/lang/String;)V");
        env->CallVoidMethod(_jobject, javaCallback, jstr);

        if(isAttached) g_jvm->DetachCurrentThread();
        logVerbose("X");
    };

    void SetJavaClazz(jclass jz, jobject jo ) {
        _jclass = jz;
        _jobject = jo;
    }

private :
    jclass  _jclass{};
    jobject _jobject{};

    B3DNativeProcessError nativehErr;
};

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_camera_B3DDepthCamera_SetFrameJNI(JNIEnv *env, jobject instance,
                                                                    jbyteArray byteData,
                                                                    jlong frameCount,
                                                                    jlong timestamp,
                                                                    jint mResolution,
                                                                    jboolean isCapture) {
    logVerbose("E");
    jbyte *javaByteData = env->GetByteArrayElements(byteData, nullptr);


    int COL_M = 2448;
    int ROW_M = 3264;

    b3dCameraPtr->decodeFrames(javaByteData,frameCount,(int64_t)timestamp,(int32_t)mResolution,isCapture, COL_M,ROW_M);

    env->ReleaseByteArrayElements(byteData, javaByteData, 0);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_camera_DepthCameraImpl_DepthCameraJNI(JNIEnv *env,
                                                                          jobject instance,
                                                                          jint camtype) {
    logVerbose("E");
    // Create a native depth camera
    b3dCameraPtr = B3DCamera::newB3DCamera((B3DCamera::DCamType)(int)camtype);

    /* Create a "stitcherProcessListener" that do stitcher process */
    cameraStreamProcessListener = make_shared<CameraStreamProcessListener>();
    /* register cameraptr so can stop the camera after process done */
    cameraStreamProcessListener->registerCameraPtr(b3dCameraPtr);

    /* Create a "CameraStreamProcessor" that cache frames and compute depth */
    cameraStreamProcessorPtr = CameraStreamProcessor::newCameraStreamProcessor();
    cameraStreamProcessorPtr->registerProcessListener(cameraStreamProcessListener);

    /* Create a stream listener that will */
    /* 1. receive frames from DepthCamera */
    /* 2. Send frames to StreamProcessor / Preview */
    streamListenerPtr = make_shared<PreviewStreamListener>();
    streamListenerPtr->registerStreamProcessor(cameraStreamProcessorPtr);

    // Register stream listener to receive frames from B3DCamera
    b3dCameraPtr->registerStreamListener(streamListenerPtr);

    isSaveResultFrame = false;
    cameraStreamProcessorPtr->setDebugFlag(isSaveResultFrame);
    logVerbose("X");
}


extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_camera_DepthCameraImpl_registerStreamListenerJNI(JNIEnv *env,
                                                                                      jobject instance,
                                                                                      jobject streamListener) {
    logVerbose("E");
    // TODO
    logVerbose("X");

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_camera_DepthCameraImpl_setPreviewTargetJNI(JNIEnv *env,
                                                                                jclass type,
                                                                                jobject dstL,
                                                                                jobject dstR,
                                                                                jobject dstM,
                                                                                jobject derivedInstance) {
    logVerbose("E");

    env->DeleteGlobalRef(g_surfaceJavaL);
    env->DeleteGlobalRef(g_surfaceJavaR);
    env->DeleteGlobalRef(g_surfaceJavaM);

    g_surfaceJavaL = env->NewGlobalRef(dstL);
    g_surfaceJavaR = env->NewGlobalRef(dstR);
    g_surfaceJavaM = env->NewGlobalRef(dstM);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_InitProcessingParemeterJNI(JNIEnv *env,
                                             jobject instance,
                                             jint processType) {
    logVerbose("E");
    int pType = (int32_t)processType;

    logVerbose("process type %d",pType);

    cameraStreamProcessorPtr->InitProcessingParameter();

    switch (pType) {
        case CameraStreamProcessor::ProcessType::PROCESS_TYPE_SINGLEVIEW :
            streamListenerPtr->startAddProcessingFrames();
            break;
        case CameraStreamProcessor::ProcessType::PROCESS_TYPE_FACE :
            streamListenerPtr->startAddFaceDetectFrames();
            break;
        case CameraStreamProcessor::ProcessType::PROCESS_TYPE_FACELANDMARK :
            streamListenerPtr->startAddGenFaceLandMarkFrames();
            break;
        case CameraStreamProcessor::ProcessType::PROCESS_TYPE_RECALIBRATION :
            streamListenerPtr->startAddRecalibrationFrames();
            break;
        case CameraStreamProcessor::ProcessType::PROCESS_TYPE_DEPTHCOMPUTATION :
            streamListenerPtr->startAddStreamDepthFrames();
            break;
        case CameraStreamProcessor::ProcessType::PROCESS_TYPE_DIAGNOSTIC :
            streamListenerPtr->startAddDiagnosticFrames();
            break;
        default:
            break;
    }
    logVerbose("X");
}


extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_StartProcessJNI(JNIEnv *env, jobject instance) {

    b3dCameraPtr->startStreamSync();

    logVerbose("E");
    if (cameraStreamProcessorPtr && !cameraStreamProcessorPtr->isProcessing()) {
        cameraStreamProcessorPtr->startProcessing();
    }
    logVerbose("X");

}


extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_StopProcessJNI(JNIEnv *env, jobject instance) {
    logVerbose("E");
    /* save frame before we clean it, cancel process will not save frames */
    if(isSaveResultFrame) {
        isSaveResultFrame = false;
        cameraStreamProcessorPtr->setDebugFlag(isSaveResultFrame);
    }

    streamListenerPtr->stopAddProcessingFrames();

    streamListenerPtr->stopAddFaceDetectFrames();

    streamListenerPtr->stopAddGenFaceLandMarkFrames();

    streamListenerPtr->stopAddRecalibrationFrames();

    streamListenerPtr->stopAddStreamDepthFrames();

    streamListenerPtr->stopAddSDiagnosticFrames();

    if (cameraStreamProcessorPtr &&
       (cameraStreamProcessorPtr->isProcessing() ||
       cameraStreamProcessorPtr->isFaceDetectProcessing() ||
       cameraStreamProcessorPtr->isFaceLandMarkProcessing() ||
       cameraStreamProcessorPtr->isRecalibrationProcessing() ||
               cameraStreamProcessorPtr->isStreamDepthProcessing())) {
        cameraStreamProcessorPtr->finishProcessing();
    }

    if( b3dStitcherPtr && b3dStitcherPtr->isStitching())
        b3dStitcherPtr->finishStitcher();

    logVerbose("X");

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_CancelProcessJNI(JNIEnv *env, jobject instance) {
    logVerbose("E");

    if(b3dCameraPtr) b3dCameraPtr->setIsCancel(true);

    streamListenerPtr->stopAddProcessingFrames();

    streamListenerPtr->stopAddFaceDetectFrames();

    streamListenerPtr->stopAddGenFaceLandMarkFrames();

    streamListenerPtr->stopAddRecalibrationFrames();

    streamListenerPtr->stopAddStreamDepthFrames();

    streamListenerPtr->stopAddSDiagnosticFrames();

    bool isCameraProcessing = true;
    bool isStitching = true;
    if (cameraStreamProcessorPtr &&
        (cameraStreamProcessorPtr->isProcessing() ||
         cameraStreamProcessorPtr->isFaceDetectProcessing() ||
         cameraStreamProcessorPtr->isFaceLandMarkProcessing() ||
         cameraStreamProcessorPtr->isRecalibrationProcessing() ||
                cameraStreamProcessorPtr->isStreamDepthProcessing())) {
        cameraStreamProcessorPtr->cancelProcessing();
    }  else {
        isCameraProcessing = false;
        logInfo("no nativeProcessor running");
    }

    if( b3dStitcherPtr && b3dStitcherPtr->isStitching())
        b3dStitcherPtr->cancelStitcher();
    else  {
        isStitching = false;
        logInfo("no stitcher running");
    }

    if(cameraStreamProcessorPtr && !isCameraProcessing && !isStitching)
        cameraStreamProcessListener->onProcessFinished("current no native process or doStitcherWork running ");
    logVerbose("X");

}


extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_InitNativeProcessorJNI(JNIEnv *env, jobject instance,
                                                              jobject b3d4nativeListener, jstring jPos,
                                                              jint processFrame,jboolean needManualRecalib) {

    logVerbose("E");
    g_obj = env->NewGlobalRef(b3d4nativeListener);

    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        logError("Some Error Occur !");
    }

    const char *cstr = env->GetStringUTFChars(jPos, nullptr);
    std::string camPos = std::string(cstr);
    b3dCameraPtr->setCamPosition(camPos);
    b3dCameraPtr->setIsCancel(false);
    env->ReleaseStringUTFChars(jPos, cstr);

    stitcherProcessListener = make_shared<StitcherProcessListener>();
    stitcherProcessListener->SetJavaClazz(nullptr, g_obj);

    b3dStitcherPtr = std::shared_ptr<B3DStitcher>(new B3DStitcher());
    b3dStitcherPtr->registerProcessListener(stitcherProcessListener);
    cameraStreamProcessListener->registerStitcher(b3dStitcherPtr);
    cameraStreamProcessListener->SetJavaClazz(nullptr, g_obj);
    cameraStreamProcessorPtr->initProcessParameters(ARC_ROOT_FOLDER_PATH,ARC_PROCESSING_SESSION_FOLDER,
            (int32_t)processFrame,camPos.compare("c") == 0,needManualRecalib);

    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_setDepthMapSettingsJNI(JNIEnv *env,
                                         jobject instance,
                                         jobject settings) {
    logVerbose("E");
    jclass settingClaZZ = env->GetObjectClass(settings);

    if(settingClaZZ == nullptr)
    {
        logError("settings is null ");
        return;
    }

    depthConfigPtr = std::shared_ptr<DepthConfigExt>(new DepthConfigExt());
    jfieldID intFieldID, floatFieldID, boolFieldID;

    intFieldID = env->GetFieldID(settingClaZZ,"depthCameraType","I");
    depthConfigPtr->depthCameraType = (DepthCameraType)env->GetIntField(settings , intFieldID);
    intFieldID = env->GetFieldID(settingClaZZ,"sceneMode","I");
    depthConfigPtr->sceneMode = (SCENE_MODE)env->GetIntField(settings , intFieldID);
    intFieldID = env->GetFieldID(settingClaZZ,"optimizeMode","I");
    depthConfigPtr->optimizeMode = (OPTIMIZE_MODE)env->GetIntField(settings , intFieldID);
    intFieldID = env->GetFieldID(settingClaZZ,"depthRegistration","I");
    depthConfigPtr->depthRegistration = (DEPTH_REGISTRATION)env->GetIntField(settings , intFieldID);
    floatFieldID = env->GetFieldID(settingClaZZ,"depthScale","F");
    depthConfigPtr->depthScale = env->GetFloatField(settings , floatFieldID);
    floatFieldID = env->GetFieldID(settingClaZZ,"depthUnit","F");
    depthConfigPtr->depthUnit = env->GetFloatField(settings , floatFieldID);
    intFieldID = env->GetFieldID(settingClaZZ,"ROI_X","I");
    depthConfigPtr->ROI_L.x = env->GetIntField(settings , intFieldID);
    intFieldID = env->GetFieldID(settingClaZZ,"ROI_Y","I");
    depthConfigPtr->ROI_L.y = env->GetIntField(settings , intFieldID);
    intFieldID = env->GetFieldID(settingClaZZ,"ROI_Width","I");
    depthConfigPtr->ROI_L.width = env->GetIntField(settings , intFieldID);
    intFieldID = env->GetFieldID(settingClaZZ,"ROI_Height","I");
    depthConfigPtr->ROI_L.height = env->GetIntField(settings , intFieldID);
    boolFieldID = env->GetFieldID(settingClaZZ,"useDSP","Z");
    depthConfigPtr->useDSP = env->GetBooleanField(settings , boolFieldID);
    boolFieldID = env->GetFieldID(settingClaZZ,"useIRThres","Z");
    depthConfigPtr->useIRThres = env->GetBooleanField(settings , boolFieldID);
    boolFieldID = env->GetFieldID(settingClaZZ,"useTwoDispMaps","Z");
    depthConfigPtr->useTwoDispMaps = env->GetBooleanField(settings , boolFieldID);
    boolFieldID = env->GetFieldID(settingClaZZ,"useDepthRange","Z");
    depthConfigPtr->useDepthRange = env->GetBooleanField(settings , boolFieldID);
    boolFieldID = env->GetFieldID(settingClaZZ,"useDispRange","Z");
    depthConfigPtr->useDispRange = env->GetBooleanField(settings , boolFieldID);
    boolFieldID = env->GetFieldID(settingClaZZ,"useNoiseRemoval","Z");
    depthConfigPtr->useNoiseRemoval = env->GetBooleanField(settings , boolFieldID);

    logVerbose("depthCameraType  %d",depthConfigPtr->depthCameraType);
    logVerbose("sceneMode  %d",depthConfigPtr->sceneMode);
    logVerbose("optimizeMode  %d",depthConfigPtr->optimizeMode);
    logVerbose("depthRegistration  %d",depthConfigPtr->depthRegistration);
    logVerbose("depthScale  %f",depthConfigPtr->depthScale);
    logVerbose("depthUnit  %f",depthConfigPtr->depthUnit);
    logVerbose("ROI_X  %d",depthConfigPtr->ROI_L.x );
    logVerbose("ROI_Y  %d",depthConfigPtr->ROI_L.y);
    logVerbose("ROI_Width  %d",depthConfigPtr->ROI_L.width );
    logVerbose("ROI_Height  %d",depthConfigPtr->ROI_L.height);
    logVerbose("useDSP  %d",depthConfigPtr->useDSP);
    logVerbose("useIRThres  %d",depthConfigPtr->useIRThres);
    logVerbose("useTwoDispMaps  %d",depthConfigPtr->useTwoDispMaps);
    logVerbose("useDepthRange  %d",depthConfigPtr->useDepthRange);
    logVerbose("useDispRange  %d",depthConfigPtr->useDispRange);
    logVerbose("useNoiseRemoval  %d",depthConfigPtr->useNoiseRemoval);

    if(cameraStreamProcessorPtr)
        cameraStreamProcessorPtr->updateProcessorSettings(depthConfigPtr);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_addFrameToNativeJNI(JNIEnv *env, jobject instance,
                                      jbyteArray byteData,
                                      jlong frameCount,
                                      jlong timestamp,
                                      jint mResolution,
                                      jboolean isCapture) {
    logVerbose("E");
    if(byteData == nullptr) {
        logWarning("image buffer might be null");
        return;
    }

    jbyte *javaByteData = env->GetByteArrayElements(byteData, nullptr);

    int COL_M = 2448;
    int ROW_M = 3264;

    // mResolution :
    // CAPTURE_SINGLE      0
    // CAPTURE_REPEATE     1
    // CAPTURE_BUFFER_2M   2
    // CAPTURE_BUFFER_8M   3
    b3dCameraPtr->decodeFrames(javaByteData,frameCount,(int64_t)timestamp,(int32_t)mResolution,isCapture, COL_M,ROW_M);

    env->ReleaseByteArrayElements(byteData, javaByteData, 0);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ConfigService_LocalConfigService_setArcNativeRootPathJNI(JNIEnv *env,
                                                         jclass type,
                                                         jstring rootPath) {
    logVerbose("E");
    const char *rootFolderPath = env->GetStringUTFChars(rootPath, nullptr);

    ARC_ROOT_FOLDER_PATH.assign(rootFolderPath);

    logVerbose("set arc root folder path to %s",ARC_ROOT_FOLDER_PATH.c_str());

    env->ReleaseStringUTFChars(rootPath, rootFolderPath);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ConfigService_LocalConfigService_setArc1LandMarkPathJNI(
        JNIEnv *env, jclass type, jstring landMarkPath) {
    logVerbose("E");
    const char *landMarkFolderPath = env->GetStringUTFChars(landMarkPath, nullptr);

    ARC1_LANDMARK_FOLDER_PATH.assign(landMarkFolderPath);

    logVerbose("set arc1 landmark folder path to %s", ARC1_LANDMARK_FOLDER_PATH.c_str());

    env->ReleaseStringUTFChars(landMarkPath, landMarkFolderPath);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_setProcessingSessionFolder(JNIEnv *env,
                                             jclass type,
                                             jstring sessionFolder) {
    logVerbose("E");
    const char *sessionFolderName = env->GetStringUTFChars(sessionFolder, nullptr);

    ARC_PROCESSING_SESSION_FOLDER.assign(sessionFolderName);

    logVerbose("set arc processing session folder to %s",ARC_PROCESSING_SESSION_FOLDER.c_str());

    env->ReleaseStringUTFChars(sessionFolder, sessionFolderName);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_StartFaceDetectionProcessJNI(JNIEnv *env,
                                               jobject instance) {

    logVerbose("E");
    b3dCameraPtr->startStreamSync();

    if (!cameraStreamProcessorPtr->isFaceDetectProcessing()) {
        cameraStreamProcessorPtr->startFaceDetect();
    }
    logVerbose("X");

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_camera_B3DDepthCamera_setDecodeTypeJNI(JNIEnv *env, jclass type, jint decodeType) {
    logVerbose("E");
    logVerbose("set decode type %d", (int32_t)decodeType);
    b3dCameraPtr->setDecodeType((B3DCamera::DecodeType)((int32_t)decodeType));
    logVerbose("X");
}

void ae_read_exp_gain_param(struct ae_exposure_param *param, cmr_u32 num, struct ae_exposure_param_switch * ae_manual_param)
{
    logVerbose("E");
    cmr_u32 i = 0;
    FILE *pf = nullptr;
    pf = fopen(AE_EXP_GAIN_PARAM_FILE_NAME, "rb");
    if (pf) {
        memset((void *)param, 0, sizeof(struct ae_exposure_param) * num);
        fread((char *)param, 1, num * sizeof(struct ae_exposure_param), pf);
        fread((char *)ae_manual_param, 1, num * sizeof(struct ae_exposure_param_switch), pf);
        fclose(pf);
        pf = nullptr;

        for (i = 0; i < num; ++i) {
            logInfo("read[%d]: %d, %d, %d, %d, %d\n", i, param[i].exp_line, param[i].exp_time, param[i].dummy, param[i].gain, param[i].bv);
        }

    }
    logVerbose("X");
}

void awb_read_gain(struct awb_save_gain *cxt, cmr_u32 num)
{
    logVerbose("E");
    cmr_u32 i = 0;
    FILE *fp = nullptr;
    fp = fopen(AWB_GAIN_PARAM_FILE_NAME, "rb");
    if (fp) {
        memset((void *)cxt, 0, sizeof(struct awb_save_gain) * num);
        fread((char *)cxt, 1, num * sizeof(struct awb_save_gain), fp);
        fclose(fp);
        fp = nullptr;

        for (i = 0; i < num; ++i) {
            logInfo("[%d]: %d, %d, %d, %d\n", i, cxt[i].r, cxt[i].g, cxt[i].b, cxt[i].ct);
        }
    }
    logVerbose("X");
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_bellus3d_android_arc_b3d4client_ConfigService_LocalConfigService_GetLastAEAWBValueJNI(JNIEnv *env,
                                                      jclass type) {

    logVerbose("E");
    static struct ae_exposure_param s_bakup_exp_param[4];
    memset(&s_bakup_exp_param,0,sizeof(s_bakup_exp_param));

    static struct ae_exposure_param_switch s_ae_manual[4];
    memset(&s_ae_manual,0,sizeof(s_ae_manual));

    struct awb_save_gain s_save_awb_param[4];
    memset(&s_save_awb_param,0,sizeof(s_save_awb_param));

    ae_read_exp_gain_param(&s_bakup_exp_param[0], sizeof(s_bakup_exp_param) / sizeof(struct ae_exposure_param),&s_ae_manual[0]);
    awb_read_gain(&s_save_awb_param[0], sizeof(s_save_awb_param) / sizeof(struct awb_save_gain));

    jintArray result;
    const int infoArraySize = 6;

    jint *intArray = new jint[infoArraySize];

    result = env->NewIntArray(infoArraySize);

    intArray[0] = s_bakup_exp_param[0].exp_line;
    intArray[1] = s_bakup_exp_param[0].gain;
    intArray[2] = s_bakup_exp_param[0].exp_time;
    intArray[3] = s_save_awb_param[0].r;
    intArray[4] = s_save_awb_param[0].g;
    intArray[5] = s_save_awb_param[0].b;

    env->SetIntArrayRegion(result, 0, infoArraySize, reinterpret_cast<const jint *>(intArray));


    logInfo("exp_line %d, exp_time %d, gain %d, r_gain %d, g_gain %d, b_gain %d",intArray[0],intArray[2],intArray[1],intArray[3],intArray[4],intArray[5]);

    delete intArray;
    logVerbose("X");
    return result;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_StartFaceLandMarkJNI(JNIEnv *env,
                                       jobject instance) {
    logVerbose("E");
    b3dCameraPtr->startStreamSync();

    if (!cameraStreamProcessorPtr->isFaceLandMarkProcessing()) {
        cameraStreamProcessorPtr->startGenFaceLandMark();
    }
    logVerbose("X");

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_StartStreamDepthJNI(JNIEnv *env,
                                      jobject instance) {
    logVerbose("E");
    b3dCameraPtr->startStreamSync();

    if (!cameraStreamProcessorPtr->isStreamDepthProcessing()) {
        cameraStreamProcessorPtr->startStreamDepth();
    }
    logVerbose("X");

}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_StartSingleDepthComputation(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jbyteArray byte_data,
                                                                                       jint width,
                                                                                       jint height,
                                                                                       jfloat imageScale) {
    logVerbose("E");
    jbyte *javaByteData = env->GetByteArrayElements(byte_data, nullptr);
    if(javaByteData == nullptr) return nullptr;

    cv::Mat IRL = cv::Mat::zeros(height, width, CV_8UC1);
    cv::Mat IRR = cv::Mat::zeros(height, width, CV_8UC1);
    cv::Mat out;
    memcpy(IRL.data, javaByteData, width * height * sizeof(unsigned char));
    javaByteData += (width*height);
    memcpy(IRR.data, javaByteData, width * height * sizeof(unsigned char));
    if(cameraStreamProcessorPtr) {
        cameraStreamProcessorPtr->doSingleDepthComputation(IRL,IRR,out,imageScale);
    }

    //convert to png
#if 0
    vector<uchar> pngBuff;
    vector<int> pngParam = vector<int>(2);
    pngParam[0] = CV_IMWRITE_PNG_COMPRESSION;
    pngParam[1] = 3;
    cv::imencode(".png",out,pngBuff,pngParam);
    int bytesize = static_cast<int>(pngBuff.size());
#endif
    int bytesize = static_cast<int>(out.total() * out.elemSize());
    jbyteArray jresult = env->NewByteArray(bytesize);

    jbyte *jbytes = env->GetByteArrayElements(jresult, nullptr);
    for ( int i = 0; i < bytesize; i++ )
    {
        jbytes[i] =  out.data[i];
    }
    env->SetByteArrayRegion(jresult, 0, bytesize, jbytes );
    logVerbose("X");
    return jresult;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_FormattingService_savePNG8UC1JNI(JNIEnv *env, jclass type,
                                 jbyteArray jbuffer,
                                 jint width, jint height,
                                 jstring jpath) {
    logVerbose("E");
    jbyte *buffer = env->GetByteArrayElements(jbuffer, nullptr);
    const char *path = env->GetStringUTFChars(jpath, nullptr);
    logVerbose("save path to %s",path);
    cv::Mat frame = cv::Mat::zeros((int) height, (int) width, CV_8UC1);
    memcpy(frame.data, buffer, (int) width * (int) height * sizeof(unsigned char));

    cv::imwrite(path, frame);

    frame.release();

    env->ReleaseByteArrayElements(jbuffer, buffer, 0);
    env->ReleaseStringUTFChars(jpath, path);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_StartRecalibrationJNI(JNIEnv *env,
                                        jobject instance) {
    logVerbose("E");
    b3dCameraPtr->startStreamSync();

    if (!cameraStreamProcessorPtr->isRecalibrationProcessing()) {
        cameraStreamProcessorPtr->startRecalibration();
    }
    logVerbose("X");
}


extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_LogService_LogService_setLogPathJNI(JNIEnv *env,
                                                                             jclass type,
                                                                             jstring path,
                                                                             jint nativeLogLevel) {
    logVerbose("E");
    const char *logpath = env->GetStringUTFChars(path, nullptr);

    /* update native log level */
    TLog::setLogLevel((TLog::LogType) ((int32_t) nativeLogLevel));
    /* update native log file path*/
    TLog::setLogFile(logpath,false);

    logVerbose("logpPath : %s loglevel %d",logpath,((int32_t) nativeLogLevel));
    env->ReleaseStringUTFChars(path, logpath);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ConfigService_LocalConfigService_updateSaveResultFlagJNI(
        JNIEnv *env, jclass type, jboolean debug) {

    logVerbose("E");
    isSaveResultFrame = (debug == JNI_TRUE);
    cameraStreamProcessorPtr->setDebugFlag(isSaveResultFrame);
    logVerbose("isSaveResultFrame %d",isSaveResultFrame);
    logVerbose("X");
}


extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_RecordingService_generatePreviewLandMarkJNI(JNIEnv *env,
                                                                                     jobject instance,
                                                                                     jbyteArray jarc1PreviewFrame) {
    logVerbose("E");
    jbyte *arc1PreviewFrame = env->GetByteArrayElements(jarc1PreviewFrame, nullptr);
    int COL_M = 1224;
    int ROW_M = 1632;
    int bytesAvailable_M = COL_M*ROW_M*3/2;

    cv::Mat frameYUV = cv::Mat::zeros(ROW_M * 3 / 2, COL_M, CV_8UC1);
    cv::Mat frameRGB;

    memcpy(frameYUV.data, (unsigned char *) arc1PreviewFrame,
           bytesAvailable_M * sizeof(unsigned char));
    cv::cvtColor(frameYUV, frameRGB, CV_YUV2BGR_NV21);
    std::thread generate(generatePreviewLandMark,frameRGB.clone());
    generate.detach();

    env->ReleaseByteArrayElements(jarc1PreviewFrame, arc1PreviewFrame, 0);
    logVerbose("X");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bellus3d_android_arc_b3d4client_ProcessingService_updateStitcherParameterJNI(JNIEnv *env,
                                             jobject instance,
                                             jdoubleArray faceRect,
                                             jdoubleArray faceDistance,
                                             jdoubleArray xForm) {
    logVerbose("E");

    if(faceRect == nullptr || faceDistance == nullptr || xForm == nullptr) {
        logWarning("stitcher setting might be null");
        return;
    }

    jdouble *jfaceRect = env->GetDoubleArrayElements(faceRect, nullptr);
    jdouble *jfaceDistance = env->GetDoubleArrayElements(faceDistance, nullptr);
    jdouble *jxForm = env->GetDoubleArrayElements(xForm, nullptr);

    int sizeFR = env->GetArrayLength(faceRect);
    int sizeFD = env->GetArrayLength(faceDistance);
    int sizeXF = env->GetArrayLength(xForm);
    logVerbose("sizeFR : %d, sizeFD : %d, sizeXF : %d",sizeFR ,sizeFD,sizeXF);

    float *faceR = new float[sizeFR];
    float *faceF = new float[sizeFD];
    for(int x = 0 ; x < sizeFR ; x++) faceR[x] = static_cast<float>(jfaceRect[x]);
    for(int x = 0 ; x < sizeFD ; x++) faceF[x] = static_cast<float>(jfaceDistance[x]);

    if(sizeXF == 16) {
        trimesh::xform xf(static_cast<double>(jxForm[0]),  static_cast<double>(jxForm[1]),   static_cast<double>(jxForm[2]),  static_cast<double>(jxForm[3]),   \
                          static_cast<double>(jxForm[4]),  static_cast<double>(jxForm[5]),   static_cast<double>(jxForm[6]),  static_cast<double>(jxForm[7]),   \
                          static_cast<double>(jxForm[8]),  static_cast<double>(jxForm[9]),   static_cast<double>(jxForm[10]), static_cast<double>(jxForm[11]), \
                          static_cast<double>(jxForm[12]), static_cast<double>(jxForm[13]),  static_cast<double>(jxForm[14]), static_cast<double>(jxForm[15]));

        b3dStitcherPtr->initStitcherInputHeadMask(faceR,
                                                  faceF,
                                                  xf);
    } else {
        trimesh::xform xf(static_cast<double>(jxForm[0]), static_cast<double>(jxForm[1]),   static_cast<double>(jxForm[2]), 0,   \
                          static_cast<double>(jxForm[3]), static_cast<double>(jxForm[4]),   static_cast<double>(jxForm[5]), 0,   \
                          static_cast<double>(jxForm[6]), static_cast<double>(jxForm[7]),   static_cast<double>(jxForm[8]), 0);

        b3dStitcherPtr->initStitcherInputHeadMask(faceR,
                                                  faceF,
                                                  xf);
    }

    delete faceR;
    delete faceF;

    env->ReleaseDoubleArrayElements(faceRect, jfaceRect, 0);
    env->ReleaseDoubleArrayElements(faceDistance, jfaceDistance, 0);
    env->ReleaseDoubleArrayElements(xForm, jxForm, 0);
    logVerbose("X");
}