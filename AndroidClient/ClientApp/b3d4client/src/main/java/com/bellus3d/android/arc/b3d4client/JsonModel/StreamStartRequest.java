package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

import java.util.List;

public class StreamStartRequest extends Request {

    /**
     * request : stream_start
     * request_id : 1580811948448-42C5F268CFD0E02454F1B53538E0DC96
     * source : COLOR
     * format : JPEG
     * request_fps : 10
     * dimension : 480x640
     * frames : 0
     * stats : ["COLOR"]
     * tracking : ["FACE"]
     */

    private String source;
    private String format;
    @SerializedName("request_fps")
    private int requestFps;
    private String dimension;
    private int frames;
    private List<String> stats;
    private List<String> tracking;
    //deprecate or reserve
    private double compression;
    @SerializedName("mode")
    private String stream_mode;
    private String irexp;
    private boolean diagnostic;

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

    public int getRequestFps() {
        return requestFps;
    }

    public void setRequestFps(int requestFps) {
        this.requestFps = requestFps;
    }

    public String getDimension() {
        return dimension;
    }

    public void setDimension(String dimension) {
        this.dimension = dimension;
    }

    public int getFrames() {
        return frames;
    }

    public void setFrames(int frames) {
        this.frames = frames;
    }

    public List<String> getStats() {
        return stats;
    }

    public void setStats(List<String> stats) {
        this.stats = stats;
    }

    public List<String> getTracking() {
        return tracking;
    }

    public void setTracking(List<String> tracking) {
        this.tracking = tracking;
    }

    public double getCompression() {
        return compression;
    }

    public void setCompression(double compression) {
        this.compression = compression;
    }

    public String get_stream_mode() {
        return stream_mode;
    }

    public void set_stream_mode(String strea_mode) {
        this.stream_mode = strea_mode;
    }

    public String getIrExp() { return irexp; }

    public void setIrExp(String irexp) {
        this.irexp = irexp;
    }

    public boolean getDiagnostic() { return diagnostic; }

    public void setDiagnostic(boolean diagnostic) {
        this.diagnostic = diagnostic;
    }
}
