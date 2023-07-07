package com.bellus3d.android.arc.b3d4client.camera;


import android.content.Context;
import android.os.Build;
import android.support.annotation.NonNull;
import android.util.Size;
import android.view.TextureView;

import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.util.ArrayList;

import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_OTHER_ERROR.INTERNAL_IMPLEMENTATION_ERROR;

/**
 * DepthCamera that streams Depth Frames
 */
public class B3DDepthCamera {

    private static final String TAG = "B3D4NewClient-B3DDepthCamera";

    private DepthCameraImpl impl;  // A pointer to the actual depth camera implementation class

    private DeviceType _DepthCamType = null ; //indicate depth camera type using

    // Internal enum for creating camera capture session
    enum CaptureSessionMode {
        PREVIEW,
        STREAM
    }

    enum PreviewStatus {
        STOPPED,
        STARTING,
        ONGOING,
        STOPPING  // maybe we can get rid of the STOPPING status
    }

    /**
     * DepthCamera device type
     */
    public enum DeviceType {
        DEVICE_B3D4,
        DEVICE_B3D4_SINGLE,
        DEVICE_FILES,
    }

    public enum Control {
        CONTROL_ON,  // 0
        CONTROL_OFF  // 1
    }

    public enum CaptureEvent {
        CAPTURE_SINGLE,
        CAPTURE_REPEATE,
        CAPTURE_BUFFER_2M,
        CAPTURE_BUFFER_8M
    }

    /* B3DCamera::DecodeType */
    public enum DecodeType {
        CALIBRATIONTOOL,   // decode order : color, IRL, IRR
        FACEDETECTION,     // decode order : color, IRL, IRR
        SINGLEVIEW,        // decode order : first 5 frames is color, IRL,IRR, rest is IRL,IRR
        FACELANDMARK,      // decode order : color, IRL, IRR
        RECALIBRATION,
        DEPTHCOMPUTATION,  // decode order :  IRL, IRR
        DIAGNOSTIC         // decode from file
    };

    public enum TripleCamResolution {
        RESOLUTION_2M,
        RESOLUTION_8M
    }

    // Load JNI .so library to class B3DDepthCamera
    static {
        // Called when class is loaded, before constructor is called
        try {
            System.loadLibrary("b3d4clientJNI");
        } catch (UnsatisfiedLinkError ule) {
            LogService.e(TAG, "Could not load native library!: " + ule.getMessage());
        }
    }

    /**
     * This method will become public for developers
     * @return An array list of String that represents supported depth camera devices
     */
    private static ArrayList<String> detectSupportedDevices() {

        ArrayList<String> supportedDevices = new ArrayList<>();

        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;

        LogService.d(TAG, "manufacturer, model: " + manufacturer + ", " + model);

        // Always supports add-on B3D4 cameras
        supportedDevices.add("ARC");

        for (String s : supportedDevices) {
            LogService.d(TAG, "Supported devices: " + s);
        }

        return supportedDevices;
    }


    /**
     * Constructor that creates a Bellus3D camera instance
     */
    public B3DDepthCamera(final Context context, DeviceType camtype) {

        _DepthCamType = camtype;

        switch (_DepthCamType) {
            case DEVICE_B3D4:
                impl = new DepthCamera_B3D4(context,_DepthCamType);
                LogService.i(TAG, "Using B3D4 built-in DepthCamera");
                break;
            case DEVICE_B3D4_SINGLE:
                impl = new DepthCamera_B3D4_SINGLE(context,_DepthCamType);
                LogService.i(TAG, "Using B3D4 built-in Single DepthCamera");
                break;
            case DEVICE_FILES:
                impl = new DepthCamera_Files(context,_DepthCamType);
                LogService.i(TAG, "Using Camera API2 DepthCamera");
                break;
            default:
                LogService.e(TAG, "Unsupported DepthCamera Device: " + camtype.toString());
                break;
        }
    }


    /**
     * Package private default constructor
     */
    B3DDepthCamera() {
        // does nothing
    }

    public DeviceType getDepthCamType() {
        return _DepthCamType;
    }

    /**
     * Pass in depth processing related settings
     * @param depthMapSettings a copy will be created inside depth camera
     */
    public void setDepthMapSettings(DepthMapSettings depthMapSettings) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return;
        }
        impl.setDepthMapSettings(depthMapSettings);
    }

    /**
     * Pass in source, format, size, and frame-rate settings
     */
    public void setCaptureSettings(B3DCaptureSettings mCaptureSetting) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return;
        }
        impl.setCaptureSettings(mCaptureSetting);
    }

    public B3DCaptureSettings getCaptureSettings() {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return null;
        }

        return impl.getCaptureSettings(_DepthCamType);
    }




    /**
     * Depth map and IR frames will be streamed out via this listener
     * @param listener continuously getting frames from depth camera
     */
    public void registerStreamListener(@NonNull DepthCameraStreamListener listener) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return;
        }
        impl.registerStreamListener(listener);
    }


    public void registerObserver(DepthCameraObserver observer) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return;
        }
        impl.registerObserver(observer);
    }

    public void unregisterStreamListener(@NonNull DepthCameraStreamListener listener) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return;
        }
        impl.unregisterStreamListener(listener);
    }


    public B3D4ClientError setPreviewTexture(final TextureView previewTexture, final TextureView testTextureView) {

        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return new B3D4ClientError(INTERNAL_IMPLEMENTATION_ERROR.getValue());
        }

        return impl.setPreviewTexture(previewTexture, testTextureView);
    }

    /**
     * Set the preview target for each camera, including depth cameras and color camera
     * @param viewL View for left depth camera
     * @param viewR View for right depth camera
     * @param viewM View for middle color camera
     */
    public void setPreviewTarget(
            final TextureView viewL, final TextureView viewR, final TextureView viewM) {
        impl.setPreviewTarget(viewL, viewR, viewM);
    }

    /**
     * Valid prior states: Connected.
     * Changes to Opening, then Open state.
     *
     * camera resource is locked after calling open(), takes more than 1 second
     * after this call, camera is ready to capture images.
     * should make sure depth camera is "OPEN" before passing to HeadScanner.
     */
    public B3D4ClientError openSync() {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return new B3D4ClientError(INTERNAL_IMPLEMENTATION_ERROR.getValue());
        }

        return impl.openSync();
    }


    /**
     * Valid prior states: Open, Opening, Streaming.
     * Changes to Closing, then Closed state.
     *
     * camera resource is released after calling close(), takes less than 1 second
     * should be called before you pause or quit the APP.
     */
    public B3D4ClientError closeSync() {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return new B3D4ClientError(INTERNAL_IMPLEMENTATION_ERROR.getValue());
        }
        return impl.closeSync();
    }

    /**
     * Starts streaming
     * @return an instance of DepthCameraError, the error information
     */
    public B3D4ClientError startStreamSync() {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return new B3D4ClientError(INTERNAL_IMPLEMENTATION_ERROR.getValue());
        }

        return impl.startStreamSync();
    }


    /**
     * Stops streaming
     * @return an instance of DepthCameraError, the error information
     */
    public B3D4ClientError stopStreamSync() {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return new B3D4ClientError(INTERNAL_IMPLEMENTATION_ERROR.getValue());
        }
        return impl.stopStreamSync();
    }

    public B3D4ClientError setCaptureEvent(B3DCaptureSettings captureSetting, CaptureEvent CEvent) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return new B3D4ClientError(INTERNAL_IMPLEMENTATION_ERROR.getValue());
        }
        return impl.setCaptureEvent(captureSetting, CEvent);
    }

    /* need to set befor openSync */
    public void setCameraID(String cameraID) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return;
        }
        impl.setCameraID(cameraID);
    }

    public Size getSize() {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return new Size(0,0);
        }
        return impl.getSize();
    }

    public long getCaptureMilliseconds() {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return 0;
        }
        return impl.getCaptureMilliseconds();
    }

    public long getProcessMilliseconds() {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return 0;
        }
        return impl.getProcessMilliseconds();
    }

    public boolean setFlood(Control ctrl, String value) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return false;
        }
        return impl.setFlood(ctrl, value);
    }

    public boolean setProject(Control ctrl, String value) {
        if (impl == null) {
            LogService.e(TAG, "implementation is null");
            return false;
        }
        return impl.setProject(ctrl, value);
    }

    /* set Java frame to native side  */
    public static native void SetFrameJNI(byte[] byteData, long frameCount, long timestamp, int mResolution, boolean isCapture);

    public static native void setDecodeTypeJNI(int decodeType);
}
