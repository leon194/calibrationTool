package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.StreamStats;
import com.google.gson.annotations.SerializedName;

public class StreamDepthRequest extends BufferIdRequest{

    /**
     * buffer_id : xxx
     * streamDepth_frame_fps : 5
     * streamDepth_frame_number : 40
     * streamDepth_color_frame_ratio : 0.5
     */

    @SerializedName("streamDepth_frame_fps")
    private int frameFPS;
    @SerializedName("frames")
    private int frameNumber;
    @SerializedName("streamDepth_color_frame_ratio")
    private double colorFrameRatio;
    private StreamStats adjust;
    private String irexp;
    private String start_index;

    public int getFrameFPS() {
        return frameFPS;
    }

    public void setFrameFPS(int frameFPS) {
        this.frameFPS = frameFPS;
    }

    public int getFrameNumber() {
        return frameNumber;
    }

    public void setFrameNumber(int frameNumber) {
        this.frameNumber = frameNumber;
    }

    public double getColorFrameRatio() {
        return colorFrameRatio;
    }

    public void setColorFrameRatio(double colorFrameRatio) {
        this.colorFrameRatio = colorFrameRatio;
    }

    public StreamStats getAdjust() {
        return adjust;
    }

    public void setAdjust(StreamStats adjust) {
        this.adjust = adjust;
    }

    public String getIrExp() {
        return irexp;
    }

    public void setIrExp(String irexp) {
        this.irexp = irexp;
    }

    public String getStartIndex() {
        return start_index;
    }

    public void setStartIndex(String startIndex) {
        this.start_index = startIndex;
    }
}
