package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class SnapShotReceiveRequest extends Request {

    //add parameter for snapshot
    @SerializedName("request_type")
    private String requestType; //preview or snapshot
    @SerializedName("frame_type")
    private String frameType; //color, L or R
    private String position; //l1, r1, l2, r2, c, t1, b1
    @SerializedName("file_nameL")
    private String fileNameL; // ex: timeStamp
    @SerializedName("file_nameR")
    private String fileNameR; // ex: timeStamp
    @SerializedName("file_nameM")
    private String fileNameM; // ex: timeStamp
    @SerializedName("buffer_id")
    private String bufferId;
    @SerializedName("buffer_finish")
    private String bufferFinish;
    @SerializedName("filesize")
    private String fileSize;

    //snapshot

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }

    public String getRequestType() {
        return requestType;
    }

    public void setRequestType(String requestType) {
        this.requestType = requestType;
    }

    public String getFrameType() {
        return frameType;
    }

    public void setFrameType(String frameType) {
        this.frameType = frameType;
    }

    public String getPosition() {
        return position;
    }

    public void setPosition(String position) {
        this.position = position;
    }

    public String getFileNameL() {
        return fileNameL;
    }

    public void setFileNameL(String fileName) {
        this.fileNameL = fileName;
    }

    public String getFileNameR() {
        return fileNameR;
    }

    public void setFileNameR(String fileName) {
        this.fileNameR = fileName;
    }

    public String getFileNameM() {
        return fileNameM;
    }

    public void setFileNameM(String fileName) {
        this.fileNameM = fileName;
    }

    public String getBufferFinish() {
        return bufferFinish;
    }

    public void setBufferFinish(String bufferFinish) {
        this.bufferFinish = bufferFinish;
    }

    public String getFileSize() {
        return fileSize;
    }

    public void setFileSize(String fileSize) {
        this.fileSize = fileSize;
    }
}
