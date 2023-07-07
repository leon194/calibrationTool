package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class SnapShotRequest extends BufferIdRequest {

    @SerializedName("ir_frame_number")
    private String IRFrameNumber;
    @SerializedName("frame_rate")
    private String frameRate;

    public String getIRFrameNumber() {
        return IRFrameNumber;
    }

    public void setIRFrameNumber(String IRFrameNumber) {
        this.IRFrameNumber = IRFrameNumber;
    }

    public String getFrameRate() {
        return frameRate;
    }

    public void setFrameRate(String frameRate) {
        this.frameRate = frameRate;
    }
}
