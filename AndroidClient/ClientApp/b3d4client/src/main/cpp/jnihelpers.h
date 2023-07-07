/*******************************************************************************
                 Helper functions for JNI Bellus3D C++ library
*******************************************************************************/

#pragma once

#include <jni.h>

#include <opencv2/opencv.hpp>

#include <b3ddepth/utils/TLog.h>

namespace b3d4 {

//// Cached global variables
extern JavaVM* g_jvm;

extern jobject g_jobj;

extern "C" {

jint JNI_OnLoad(JavaVM *jvm, void *reserved);

}  // End of extern "C"


// ==== Helper functions for getting native handler
jfieldID getHandleField(JNIEnv *env, jobject obj);

template <typename T>
T *getHandle(JNIEnv *env, jobject obj) {
    if (obj == nullptr) {
        logError(" jobject is nullptr");
        return nullptr;
    }
    jlong handle = env->GetLongField(obj, getHandleField(env, obj));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t) {

    if (obj == nullptr) {
        logError(" jobject is nullptr");
        return;
    }

    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj), handle);
}

// Helper functions for rendering texture view from native code
    void renderSurfaceTexture(JNIEnv *env, jobject dstSurface, const cv::Mat frame);


// ==== Helper functions for native thread calling Java methods
// call this before calling Java methods from C++ thread
JNIEnv* getJNIEnv(JavaVM* vm, bool& needsDetach);

// detach thread if "needsDetach = true"
void detachJNI(JavaVM* vm);


}  // End of namespace b3d4
