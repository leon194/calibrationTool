package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class TimeStampNotification extends Notification {
    private String capture_type;
    @SerializedName("index")
    private String frameIndex;
    @SerializedName("l")
    private String timeStampL;
    @SerializedName("r")
    private String timeStampR;

    public String getCaptureType() { return capture_type;}
    public void setCaptureType(String capture_type) {
        this.capture_type = capture_type;
    }

    public String getFrameIndexIndex() {return frameIndex;}
    public void setFrameIndex(String frameIndex) {
        this.frameIndex = frameIndex;
    }

    public String getTimeStampL() {return timeStampL;}
    public void setTimeStampL(String timeStampL) {
        this.timeStampL = timeStampL;
    }

    public String getTimeStampR() {return timeStampR;}
    public void setTimeStampR(String timeStampR) {
        this.timeStampR = timeStampR;
    }
}
