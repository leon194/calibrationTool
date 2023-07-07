package com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel;

import com.google.gson.annotations.SerializedName;

public class Tracking {

    @SerializedName("FACE")
    private FaceTrackingInfo faceTrackingInfo;

    public FaceTrackingInfo getFaceTrackingInfo() {
        return faceTrackingInfo;
    }

    public void setFaceTrackingInfo(FaceTrackingInfo faceTrackingInfo) {
        this.faceTrackingInfo = faceTrackingInfo;
    }
}
