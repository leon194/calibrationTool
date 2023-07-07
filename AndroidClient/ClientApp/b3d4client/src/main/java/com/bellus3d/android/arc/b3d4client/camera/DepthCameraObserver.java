package com.bellus3d.android.arc.b3d4client.camera;

/**
 * Classes that observe DepthCamera should extends this class
 */

public interface DepthCameraObserver {

    void onUpdate(DepthCameraState.StateType depthCameraState);

    void onError(DepthCameraError depthCameraError);
}
