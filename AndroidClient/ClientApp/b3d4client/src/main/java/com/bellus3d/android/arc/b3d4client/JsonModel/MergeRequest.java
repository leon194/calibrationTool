package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class MergeRequest {

    /**
     * ts : 20120101005217694
     * request : merge
     * buffer_id : 20200203112355639
     * request_id : 20200203112402297
     * filename : c.zip
     * filesize : 2341907
     */

    private String ts;
    private String request;
    @SerializedName("buffer_id")
    private String bufferId;
    @SerializedName("request_id")
    private String requestId;
    private String filename;
    @SerializedName("filesize")
    private String fileSize;
    private String recalibType;
    private String caldispError;
    private String is_recalibration_success;

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

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getFilename() {
        return filename;
    }

    public void setFilename(String filename) {
        this.filename = filename;
    }

    public String getFileSize() {
        return fileSize;
    }

    public void setFileSize(String fileSize) {
        this.fileSize = fileSize;
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
