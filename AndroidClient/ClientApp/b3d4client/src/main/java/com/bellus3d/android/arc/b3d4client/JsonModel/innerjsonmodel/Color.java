package com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel;

import com.google.gson.annotations.SerializedName;

public class Color {

    /**
     * y : 786
     * r : 1659
     * g : 1024
     * b : 1658
     */

    private String y;
    private String r;
    private String g;
    private String b;
    @SerializedName("exp_line")
    private String exposureLine;
    @SerializedName("exp_time")
    private String exposureTime;

    public String getY() {
        return y;
    }

    public void setY(String y) {
        this.y = y;
    }

    public String getR() {
        return r;
    }

    public void setR(String r) {
        this.r = r;
    }

    public String getG() {
        return g;
    }

    public void setG(String g) {
        this.g = g;
    }

    public String getB() {
        return b;
    }

    public void setB(String b) {
        this.b = b;
    }

    public String getExposureLine() {
        return exposureLine;
    }

    public void setExposureLine(String exposureLine) {
        this.exposureLine = exposureLine;
    }

    public String getExposureTime() {
        return exposureTime;
    }

    public void setExposureTime(String exposureTime) {
        this.exposureTime = exposureTime;
    }
}
