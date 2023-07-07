package com.bellus3d.android.arc.b3d4client.camera;

/**
 *  Implement in B3DCameraService.java
 *  pass this Listener to JNI, will notice
 *  FrameEnough, ProcessDone and pass byte array back
 */

import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;

import java.io.IOException;

public abstract class B3D4NativeProcessorListener {

    /* notify an Error */
    public void onError(int errorcode) {};

    public void onError(B3D4ClientError error, B3DCaptureSettings captureSetting) {};

    /* notify that depth frame is enough */
    public void onFrameEnough() {};

    /* notify that stitcher process done */
    public void onProcessDone(byte[] bData) throws IOException {};

    /* report head post info */
    public void onFaceDetectDone(float[] headPoseInfo) {};

    /* report generate Face LandMark done */
    public void onGenFaceLandMarkDone() {};

    /* report recalibration success or not  */
    public void onRecalibrationDone(int errorcode, float calibDispErr) {};

    /* report a depth computation frame */
    public void onStreamDepthDone(byte[] bData, long timeStampL) {};

    /* report native process finish done */
    public void onProcessFinished(String who) {};

}

