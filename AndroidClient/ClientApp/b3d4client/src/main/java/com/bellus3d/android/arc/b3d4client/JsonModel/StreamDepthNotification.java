package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class StreamDepthNotification extends Notification {
    // {  notification:  "stream_depth",   capture_completed: true,  }
    /**
     * capture_completed : true
     */
    @SerializedName("capture_completed")
    private Boolean captureCompleted;
    @SerializedName("buffer_id")
    private String bufferId;

    public Boolean getCaptureCompleted() {
        return captureCompleted;
    }

    public void setCaptureCompleted(Boolean captureCompleted) {
        this.captureCompleted = captureCompleted;
    }

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }
}
