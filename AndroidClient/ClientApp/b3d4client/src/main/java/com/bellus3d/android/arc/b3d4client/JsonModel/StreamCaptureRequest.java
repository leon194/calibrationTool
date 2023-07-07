package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.HeadPostTrackInfo;
import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.YamlData;
import com.google.gson.annotations.SerializedName;

public class StreamCaptureRequest extends Request {

    /**
     * frame_type : combine
     * filename : 001_000204_00001
     * filesize : 6291456
     * buffer_finish : false
     * frame_index : 1
     */
    @SerializedName("buffer_id")
    private String bufferId;
    @SerializedName("frame_type")
    private String frameType;
    private String filenameLIR;
    private String filenameRIR;
    private String filenameColor;
    private String filenameDepth;
    private String filesize;
    @SerializedName("buffer_finish")
    private String bufferFinish;
    @SerializedName("frame_index")
    private String frameIndex;
    private HeadPostTrackInfo headrotation;
    private YamlData data;

    private String recalibType;
    private String caldispError;
    private String is_recalibration_success;

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }

    public String getFrameType() {
        return frameType;
    }

    public void setFrameType(String frameType) {
        this.frameType = frameType;
    }

    public String getFilenameLIR() {
        return filenameLIR;
    }

    public void setFilenameLIR(String filename) {
        this.filenameLIR = filename;
    }

    public String getFilenameRIR() {
        return filenameRIR;
    }

    public void setFilenameRIR(String filename) {
        this.filenameRIR = filename;
    }

    public String getFilenameColor() {
        return filenameColor;
    }

    public void setFilenameColor(String filename) {
        this.filenameColor = filename;
    }

    public String getFilenameDepth() {
        return filenameDepth;
    }

    public void setFilenameDepth(String filename) {
        this.filenameDepth = filename;
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

    public String getFrameIndex() {
        return frameIndex;
    }

    public void setFrameIndex(String frameIndex) {
        this.frameIndex = frameIndex;
    }

    public HeadPostTrackInfo getHeadPost() {
        return headrotation;
    }

    public void setHeadPost(HeadPostTrackInfo headrotation) {
        this.headrotation = headrotation;
    }

    public YamlData getData() {
        return data;
    }

    public void setData(YamlData data) {
        this.data = data;
    }

    public String getRecalibType() {
        return recalibType;
    }

    public void setRecalibType(String recalibType) {
        this.recalibType = recalibType;
    }

    public String getCaldispError() {
        return caldispError;
    }

    public void setCaldispError(String caldispError) {
        this.caldispError = caldispError;
    }

    public String getCalibResult() {
        return is_recalibration_success;
    }

    public void setCalibResult(String reCalibResult) {
        this.is_recalibration_success = reCalibResult;
    }
}
