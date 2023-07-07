package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class SendCaptureDataRequest extends Request{

    /**
     * status : OK
     * filesize : 24056
     * camera : c
     * device_id : BL1909070077
     * request : buffer_capture
     * request_id : 1580470753021-071F98EB62BBFA7931733592AD6C3924
     */

    private String status;
    @SerializedName("filesize")
    private String fileSize;
    private String camera;
    @SerializedName("device_id")
    private String deviceId;
    @SerializedName("buffer_id")
    private String bufferId;

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public String getFileSize() {
        return fileSize;
    }

    public void setFileSize(String fileSize) {
        this.fileSize = fileSize;
    }

    public String getCamera() {
        return camera;
    }

    public void setCamera(String camera) {
        this.camera = camera;
    }

    public String getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(String deviceId) {
        this.deviceId = deviceId;
    }

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }
}
