package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

import java.util.List;

public class BufferColorRequest extends Request {

    /**
     * request : buffer_color
     * request_id : 1589256470450
     * buffer_id : 20200512120722318
     * indices : [0,-1,-1,40,13,59,25,48,19]
     * request_type :  arc1/arcx_fullhead
     */

    @SerializedName("buffer_id")
    private String bufferId;
    private List<Integer> indices;
    private String filenameColor;
    private String filesize;
    @SerializedName("buffer_finish")
    private String bufferFinish;
    @SerializedName("frame_type")
    private String frameType;
    @SerializedName("request_type")
    private String requestType;
    @SerializedName("device_id")
    private String deviceId;
    private String camera;

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferid) {
        this.bufferId = bufferid;
    }

    public List<Integer> getIndices() {
        return indices;
    }

    public void setIndices(List<Integer> indices) {
        this.indices = indices;
    }

    public String getFilenameColor() {
        return filenameColor;
    }

    public void setFilenameColor(String filenameColor) {
        this.filenameColor = filenameColor;
    }

    public String getFilesize() {
        return filesize;
    }

    public void setFilesize(String filesize) {
        this.filesize = filesize;
    }

    public String getBufferFinish() {
        return bufferFinish;
    }

    public void setBufferFinish(String bufferFinish) {
        this.bufferFinish = bufferFinish;
    }

    public String getFrameType() {
        return frameType;
    }

    public void setFrameType(String frameType) {
        this.frameType = frameType;
    }

    public String getRequestType() {
        return requestType;
    }

    public void setRequestType(String requestType) {
        this.requestType = requestType;
    }

    public String getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(String deviceId) {
        this.deviceId = deviceId;
    }

    public String getCamera() {
        return camera;
    }

    public void setCamera(String camera) {
        this.camera = camera;
    }
}
