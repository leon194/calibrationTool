package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.StreamStats;
import com.google.gson.annotations.SerializedName;

public class StreamStopResponseTo {

    /**
     * height : 544
     * streamed_frames : 71
     * request_id : 1580648817082-33D80E4DEC5F22D0C98DEC1735E86876
     * total_ms : 7734
     * captured_frames : 91
     * capture_ms : 6862
     * streamed_bytes : 8131711
     * response_to : stream_stop
     * width : 408
     * streamStats : {"color":{"y":"786","r":"1659","g":"1024","b":"1658"}}
     */

    private String height;
    @SerializedName("streamed_frames")
    private String streamedFrames;
    @SerializedName("request_id")
    private String requestId;
    @SerializedName("total_ms")
    private String totalMs;
    @SerializedName("captured_frames")
    private String capturedFrames;
    @SerializedName("capture_ms")
    private String captureMs;
    @SerializedName("streamed_bytes")
    private String streamedBytes;
    @SerializedName("response_to")
    private String responseTo;
    private String width;
    @SerializedName("stats")
    private StreamStats streamStats;

    public String getHeight() {
        return height;
    }

    public void setHeight(String height) {
        this.height = height;
    }

    public String getStreamedFrames() {
        return streamedFrames;
    }

    public void setStreamedFrames(String streamedFrames) {
        this.streamedFrames = streamedFrames;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getTotalMs() {
        return totalMs;
    }

    public void setTotalMs(String totalMs) {
        this.totalMs = totalMs;
    }

    public String getCapturedFrames() {
        return capturedFrames;
    }

    public void setCapturedFrames(String capturedFrames) {
        this.capturedFrames = capturedFrames;
    }

    public String getCaptureMs() {
        return captureMs;
    }

    public void setCaptureMs(String captureMs) {
        this.captureMs = captureMs;
    }

    public String getStreamedBytes() {
        return streamedBytes;
    }

    public void setStreamedBytes(String streamedBytes) {
        this.streamedBytes = streamedBytes;
    }

    public String getResponseTo() {
        return responseTo;
    }

    public void setResponseTo(String responseTo) {
        this.responseTo = responseTo;
    }

    public String getWidth() {
        return width;
    }

    public void setWidth(String width) {
        this.width = width;
    }

    public StreamStats getStreamStats() {
        return streamStats;
    }

    public void setStreamStats(StreamStats streamStats) {
        this.streamStats = streamStats;
    }
}
