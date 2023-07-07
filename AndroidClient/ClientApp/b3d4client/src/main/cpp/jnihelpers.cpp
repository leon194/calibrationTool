/*******************************************************************************
                 Helper functions for JNI Bellus3D C++ library
*******************************************************************************/

#include "jnihelpers.h"

#include <cstring>  // for memcpy

#include <android/native_window_jni.h>

#include <android/native_window_jni.h>


namespace b3d4 {

JavaVM* g_jvm;

jobject g_jobj;


jint JNI_OnLoad(JavaVM *jvm, void *reserved) {

//logAlways("Caching JavaVM and other references here.");

    /* init JVM*/
    g_jvm = jvm;  // cache the JavaVM pointer

    JNIEnv *env;
    int status = jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if(status < 0) {
        logError("JNI_onLoad() GetEnv failed");
    }

    return JNI_VERSION_1_6;
}


// Check before calling Java methods from C++ thread
JNIEnv* getJNIEnv(JavaVM* vm, bool& needsDetach) {

    needsDetach = false;

    JNIEnv* env;  // check if current thread has JNIEnv
    int getEnvStat = vm->GetEnv((void **)&env, JNI_VERSION_1_6);

    if (getEnvStat == JNI_EDETACHED) {  // current thread is not attached VM

        int result = vm->AttachCurrentThread(&env, nullptr);  // current thread attempt to attach VM

        if (result != JNI_OK) {
            logError("thread attach failed: %d", result);
            return nullptr;
        }
        needsDetach = true;
    }
    else if (getEnvStat == JNI_OK) {
        // means current thread is a Java thread?
        // TLog::log(TLog::LOG_VERBOSE, "current (java?)thread already attached to VM");
    }
    else {
        logError("thread GetEnv error: %d", getEnvStat);
    }
    return env;
}


// If needs detach, then call this
// only call this if needsDetach = true
void detachJNI(JavaVM* vm) {

    int result = vm->DetachCurrentThread();
    if (result != JNI_OK) {
        logError("thread detach failed: %d", result);
    }
}


// Helper functions for getting native handler
jfieldID getHandleField(JNIEnv *env, jobject obj) {

    jclass c = env->GetObjectClass(obj);

    return env->GetFieldID(c, "nativeHandle", "J");  // J is the type signature for long:
}

    void renderSurfaceTexture(JNIEnv *env, jobject dstSurface, const cv::Mat frame) {

        if (frame.empty()) return;

        ANativeWindow *win = ANativeWindow_fromSurface(env, dstSurface);
        ANativeWindow_acquire(win);
        ANativeWindow_Buffer buf;

        // TODO: check input image frame format?

        const int IMAGE_COL = frame.cols;
        const int IMAGE_ROW = frame.rows;

        ANativeWindow_setBuffersGeometry(win, IMAGE_COL, IMAGE_ROW, WINDOW_FORMAT_RGBA_8888 /*format unchanged*/);

        if (int32_t err = ANativeWindow_lock(win, &buf, NULL)) {
            logError("ANativeWindow_lock fails with error code %d", err);
            ANativeWindow_release(win);
            return;
        }

        cv::Mat dstImg;

        if (frame.channels() == 1) {
            if(frame.type() == CV_16UC1) {
                // This is a depth frame
                frame.convertTo(dstImg, CV_8U, 255.0 / 65536.0);
                cvtColor(dstImg, dstImg, CV_GRAY2RGBA);

            } else {
                // This is a IR frame
                cvtColor(frame, dstImg, CV_GRAY2RGBA);
            }

        }
        else if (frame.channels() == 3) {
            cvtColor(frame, dstImg, CV_BGR2RGBA);
        }
        else {
            logError("Image: %d, %d, %d, %d", frame.rows, frame.cols, frame.channels(), frame.type());
        }

        if (!dstImg.empty() || (dstImg.channels() != 4)) {

            // Have to copy row by row because: buf.stride != Mat.stride (number of bytes per row)
            uint8_t* buf_ptr = (uint8_t*)buf.bits;
            int bufStride = buf.stride * 4;
            int imgStride = IMAGE_COL  * 4;  // check opencv Mat is continuous?

            for (int r = 0; r < dstImg.rows; ++r) {
                memcpy(buf_ptr + r * bufStride, dstImg.ptr() + r * imgStride, (size_t)imgStride);
            }
        }
        else {
            logError("Invalid dstImg format");
        }


        ANativeWindow_unlockAndPost(win);
        ANativeWindow_release(win);
    }

}  // End of namespace b3d4
