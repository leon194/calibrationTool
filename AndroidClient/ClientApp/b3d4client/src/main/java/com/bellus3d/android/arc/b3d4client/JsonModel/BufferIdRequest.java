package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.Tracking;
import com.google.gson.annotations.SerializedName;

import java.util.List;

public class BufferIdRequest extends Request{

    /**
     * buffer_id : 20200204194627581
     */

    @SerializedName("buffer_id")
    private String bufferId;
    private boolean debug;
    private boolean manual_recalibration;
    private Tracking tracking;
    private List<String> xf;

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }

    public boolean isDebug() {
        return debug;
    }

    public void setDebug(boolean debug) {
        this.debug = debug;
    }

    public boolean isManual_recalibration() {
        return manual_recalibration;
    }

    public void setManual_recalibration(boolean manual_recalibration) {
        this.manual_recalibration = manual_recalibration;
    }

    public Tracking getTracking() {
        return tracking;
    }

    public void setTracking(Tracking tracking) {
        this.tracking = tracking;
    }

    public List<String> getXf() {
        return xf;
    }

    public void setXf(List<String> xf) {
        this.xf = xf;
    }
}
