package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class BufferIdResponseTo extends ResponseTo {

    /**
     * buffer_id : 20200131201921213
     * status : OK
     * request_id : 1580473161209-9AEB7408D8FFBF25FEAC28BB412ADCFE
     * response_to : buffer_capture
     */

    @SerializedName("buffer_id")
    private String bufferId;

    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }
}
