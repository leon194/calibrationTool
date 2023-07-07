package com.bellus3d.android.arc.b3d4client.camera;

import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.FrameBuffer;

/**
 * For sending DeviceCamera frames to listeners
 */

public abstract class DepthCameraStreamListener {

    /* a callback that return an Warning, only report an error to host without changeing the light  */
    public abstract void onWarning(B3D4ClientError error, B3DCaptureSettings captureSetting);

    /* a callback that return an Error */
    public abstract void onError(B3D4ClientError error, B3DCaptureSettings captureSetting);

    /* a callback that returns frame buffer */
    public void onFrameBuffer(B3DCaptureSettings mCaptureSetting, final FrameBuffer frameBuffer) {};

    public void onFrameBytes(B3DCaptureSettings mCaptureSetting, final byte[] byteData) {};

    public abstract void onStatusHost(B3DCaptureSettings mCaptureSetting, String status);

    public void onStatusHostSub(B3DCaptureSettings mCaptureSetting, String status) {};

    public abstract void onMsgHost(B3DCaptureSettings mCaptureSetting, String msg);

    //TODO make it more comment case
    public abstract void onNotificationHost(B3DCaptureSettings mCaptureSetting, long timeStampL, long timeStampR, long frameIndex);

    public abstract void onFrameHost(B3DCaptureSettings settings, byte[] byteData, long frameID);

    public void onFrameHostSub(B3DCaptureSettings settings, byte[] byteData, long frameID) {};

    public abstract void onFrameNative(byte[] byteData, long mTotalFramesStreamed, long currentTimeStamp, int mResolution, boolean isCapture);

}
