package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.StreamStats;
import com.google.gson.annotations.SerializedName;

public class CaptureRequest extends Request {

    /**
     * buffer_id : 20200204140643443
     * source : DEVICE
     * format : RAW
     * dimension : 240x320
     * ir_projector : true
     * ir_flood : false
     * return_first_frame : true
     * frames : 15
     * adjust : {"color":{"b":"2369","g":"1024","r":"1160","y":"384"}}
     */
    @SerializedName("buffer_id")
    private String bufferId;
    private String source;
    private String format;
    private String dimension;
    @SerializedName("ir_projector")
    private boolean irProjector;
    @SerializedName("ir_flood")
    private boolean irFlood;
    @SerializedName("return_first_frame")
    private boolean returnFirstFrame;
    private int frames;
    private StreamStats adjust;
    private String irexp;

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }

    public String getSource() {
        return source;
    }

    public void setSource(String source) {
        this.source = source;
    }

    public String getFormat() {
        return format;
    }

    public void setFormat(String format) {
        this.format = format;
    }

    public String getDimension() {
        return dimension;
    }

    public void setDimension(String dimension) {
        this.dimension = dimension;
    }

    public boolean isIrProjector() {
        return irProjector;
    }

    public void setIrProjector(boolean irProjector) {
        this.irProjector = irProjector;
    }

    public boolean isIrFlood() {
        return irFlood;
    }

    public void setIrFlood(boolean irFlood) {
        this.irFlood = irFlood;
    }

    public boolean isReturnFirstFrame() {
        return returnFirstFrame;
    }

    public void setReturnFirstFrame(boolean returnFirstFrame) {
        this.returnFirstFrame = returnFirstFrame;
    }

    public int getFrames() {
        return frames;
    }

    public void setFrames(int frames) {
        this.frames = frames;
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
}
