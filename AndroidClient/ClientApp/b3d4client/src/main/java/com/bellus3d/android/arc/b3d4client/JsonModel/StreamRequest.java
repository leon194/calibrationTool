package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.Tracking;
import com.google.gson.annotations.SerializedName;

public class StreamRequest {

    /**
     * ts : 1746495579740
     * request : stream
     * device_id : BL1909070077
     * request_id : 1580367302388-549D7FCEC59ED17983C1608619CC7B5E
     * width : 408
     * camera : c
     * height : 544
     * frame_index : 100
     * filesize : 132225
     * tracking : {"FACE":{"distance":{"x":"0.0","y":"0.0","z":"0.0"},"rotation":{"x":"0.0","y":"0.0","z":"0.0"}}}
     */

    private String ts;
    private String request;
    @SerializedName("device_id")
    private String deviceId;
    @SerializedName("request_id")
    private String requestId;
    private String width;
    private String camera;
    private String height;
    @SerializedName("frame_index")
    private String frameIndex;
    @SerializedName("filesize")
    private String fileSize;
    private Tracking tracking;

    public String getTs() {
        return ts;
    }

    public void setTs(String ts) {
        this.ts = ts;
    }

    public String getRequest() {
        return request;
    }

    public void setRequest(String request) {
        this.request = request;
    }

    public String getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(String deviceId) {
        this.deviceId = deviceId;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getWidth() {
        return width;
    }

    public void setWidth(String width) {
        this.width = width;
    }

    public String getCamera() {
        return camera;
    }

    public void setCamera(String camera) {
        this.camera = camera;
    }

    public String getHeight() {
        return height;
    }

    public void setHeight(String height) {
        this.height = height;
    }

    public String getFrameIndex() {
        return frameIndex;
    }

    public void setFrameIndex(String frameIndex) {
        this.frameIndex = frameIndex;
    }

    public String getFileSize() {
        return fileSize;
    }

    public void setFileSize(String fileSize) {
        this.fileSize = fileSize;
    }

    public Tracking getTracking() {
        return tracking;
    }

    public void setTracking(Tracking tracking) {
        this.tracking = tracking;
    }
}
