package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class StreamStopRequest extends Request {

    @SerializedName("mode")
    private String stream_mode;

    public String get_stream_mode() {
        return stream_mode;
    }

    public void set_stream_mode(String strea_mode) {
        this.stream_mode = strea_mode;
    }
}
