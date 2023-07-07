package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class BufferFetchRequest extends Request {
    @SerializedName("frame_type")
    private String frameType; //color, L or R
    @SerializedName("buffer_type")
    private int bufferType; //color(1), L(2), R(4), depth(8)
    @SerializedName("file_nameL")
    private String fileNameL; // ex: timeStamp
    @SerializedName("file_nameR")
    private String fileNameR; // ex: timeStamp
    @SerializedName("file_nameM")
    private String fileNameM; // ex: timeStamp
    @SerializedName("file_nameD")
    private String fileNameD; // ex: timeStamp
    @SerializedName("buffer_id")
    private String bufferId;
    @SerializedName("buffer_finish")
    private String bufferFinish;
    @SerializedName("filesize")
    private String fileSize;
    private String timeStamp;


    public String getFrameType() { return frameType ;}

    public void setFrameType(String frameType) { this.frameType  = frameType;}

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }

    public int getBufferType() {
        return bufferType ;
    }

    public void setBufferType(int bufferType) {
        this.bufferType  = bufferType;
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

    public String getFileNameD() {
        return fileNameD;
    }

    public void setFileNameD(String fileName) {
        this.fileNameD = fileName;
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

    public String getTimeStamp() {
        return timeStamp;
    }

    public void setTimeStamp(String timeStamp) {
        this.timeStamp = timeStamp;
    }


}
