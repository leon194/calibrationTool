package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class StreamStartResponseTo extends ResponseTo {

    /**
     * status : OK
     * response_to : stream_start
     * request_id : 1580386835417-92B4BAD3136B2A74A601C8F453706FDA
     * camera : c
     * start : 1612137642984
     * device_id : BL1909070077
     */

    private String camera;
    private String start;
    @SerializedName("device_id")
    private String deviceId;

    public String getCamera() {
        return camera;
    }

    public void setCamera(String camera) {
        this.camera = camera;
    }

    public String getStart() {
        return start;
    }

    public void setStart(String start) {
        this.start = start;
    }

    public String getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(String deviceId) {
        this.deviceId = deviceId;
    }
}
