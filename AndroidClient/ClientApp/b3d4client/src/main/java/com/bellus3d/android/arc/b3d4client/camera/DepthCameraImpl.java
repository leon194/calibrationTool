package com.bellus3d.android.arc.b3d4client.camera;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Range;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.util.ArrayList;
import java.util.List;

/**
 * DepthCamera implementation for OEM and B3D4 projects
 */
abstract class DepthCameraImpl {

    private static final String TAG = "B3D4NewClient-DepthCameraImpl";

    Context mActivityContext;

    DepthMapSettings mDepthMapSettings;

    DepthCameraState.StateType mCurrentStateType;

    HandlerThread depthCameraCaptureThread;

    HandlerThread preprocessingThread;


    Handler mCaptureHandler;  // TODO: rename the handler

    Handler mProcessingHandler;

    B3DDepthCamera.PreviewStatus mPreviewStatus;

    public long mTotalFramesStreamed;  // streamed counter
    public long mTotalFramesCaptured;  // processed frame counter
    public long mStreamStartTime;      // first stream start time
    public long mStreamAvaliableTime;  // record onImageAvailable time
    public long mStreamedTotalTime;   // record every stream time between onImageAvailable


    DepthCameraImpl(B3DDepthCamera.DeviceType _DepthCamType) {
    }


    // Implement Observer pattern
    private List<DepthCameraObserver> mObservers = new ArrayList<>();

    void setDepthMapSettings(DepthMapSettings depthMapSettings) {

        // Make a copy of the passed in settings
        mDepthMapSettings = new DepthMapSettings(depthMapSettings);

        // Set depth map settings
        setDepthMapSettingsJNI(mDepthMapSettings);
    }


    abstract B3D4ClientError setPreviewTexture(final TextureView previewTexture, final TextureView testTextureView);

    /* open init camera using this function */
    abstract B3D4ClientError openSync();

    /* close deinit camera using this function */
    abstract B3D4ClientError closeSync();

    /* enable streaming/preview using this function */
    abstract B3D4ClientError startStreamSync();

    /* disable streaming/preview using this function */
    abstract B3D4ClientError stopStreamSync();

    /* capture a image using this function  */
    abstract B3D4ClientError setCaptureEvent(B3DCaptureSettings captureSetting, B3DDepthCamera.CaptureEvent CEvent);

    /* listener that returns frame from java camera API   */
    abstract void registerStreamListener(DepthCameraStreamListener listener);

    abstract void unregisterStreamListener(DepthCameraStreamListener listener);

    abstract void setPreviewTarget(TextureView viewL, TextureView viewR, TextureView viewM);

    abstract void setCameraID(String cameraID);

    abstract Size getSize();

    abstract long getCaptureMilliseconds();

    abstract long getProcessMilliseconds();

    abstract boolean setFlood(B3DDepthCamera.Control ctrl, String value);

    abstract boolean setProject(B3DDepthCamera.Control ctrl, String value);

    /* config fps */
    abstract DepthCameraError setFps(Range<Integer> fps);

    abstract float getFps();

    abstract void setCaptureSettings(B3DCaptureSettings mCaptureSetting);

    B3DCaptureSettings getCaptureSettings(B3DDepthCamera.DeviceType _DepthCamType) {
        return new B3DCaptureSettings(_DepthCamType);
    }

    public void registerObserver(DepthCameraObserver observer) {
        mObservers.add(observer);
    }

    public void unregisterObserver(DepthCameraObserver observer) {
        mObservers.remove(observer);
    }


    void reportUpdateToAllObservers(DepthCameraState.StateType currentState) {
        for (DepthCameraObserver observer : mObservers) {
            observer.onUpdate(currentState);
        }
    }

    void reportErrorToAllObservers(DepthCameraError depthCameraError) {
        for (DepthCameraObserver observer : mObservers) {
            observer.onError(depthCameraError);
        }
    }

    public DepthCameraState.StateType getCurrentStateType() {
        return mCurrentStateType;
    }

    void reportUpdateSync(DepthCameraState.StateType currentStateType) {
        mCurrentStateType = currentStateType;
            switch (mCurrentStateType) {
                case CONNECTED: reportUpdateToAllObservers(DepthCameraState.StateType.CONNECTED);
                    break;
                case OPENING:   reportUpdateToAllObservers(DepthCameraState.StateType.OPEN);
                    break;
                case CLOSING:   reportUpdateToAllObservers(DepthCameraState.StateType.CLOSING);
                    break;
                case OPEN:      reportUpdateToAllObservers(DepthCameraState.StateType.OPEN);
                    break;
                case STARTING:  reportUpdateToAllObservers(DepthCameraState.StateType.STARTING);
                    break;
                case STOPPING:  reportUpdateToAllObservers(DepthCameraState.StateType.STOPPING);
                    break;
                case STREAMING: reportUpdateToAllObservers(DepthCameraState.StateType.STREAMING);
                    break;
                default: LogService.e(TAG, " error - " + mCurrentStateType);
                    break;
            }
    }


    void updateCameraState(DepthCameraState.StateType updatedStateType) {

        LogService.d(TAG,updatedStateType.toString());
        // update the DepthCamera member variable
        mCurrentStateType = updatedStateType;

        // report state to observers
        reportUpdateToAllObservers(mCurrentStateType);
    }


    public void InitMerge(B3D4NativeProcessorListener b3d4nativeListener, int pos, int processFrame) {
        //InitNativeProcessorJNI(b3d4nativeListener,pos, processFrame);
    }

    void startBackgroundThreads() {
        depthCameraCaptureThread = new HandlerThread("DepthCam-Thread");
        depthCameraCaptureThread.start();
        mCaptureHandler = new Handler(depthCameraCaptureThread.getLooper());

        preprocessingThread = new HandlerThread("DepthCam-Preprocess-Thread");
        preprocessingThread.start();
        mProcessingHandler = new Handler(preprocessingThread.getLooper());
    }

    void stopBackgroundThreads() {

        if(depthCameraCaptureThread != null)
            depthCameraCaptureThread.quitSafely();
        if(preprocessingThread != null)
            preprocessingThread.quitSafely();
    }

    // Native Wrappers of DepthCamera
    // static native DeviceType detectDeviceTypeJNI();

    /* init DepthCamera JNI with camera type */
    public native void DepthCameraJNI(int deviceType);

    static native void setPreviewTargetJNI(Surface dstL, Surface dstR, Surface dstM, DepthCameraImpl derivedInstance);

    private native void setDepthMapSettingsJNI(DepthMapSettings settings);

}
