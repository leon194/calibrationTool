package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class OffsetNotification extends Notification {
    @SerializedName("buffer_id")
    private String bufferId;
    @SerializedName("request_id")
    private String requestId;



    public String getBufferId() {
        return bufferId;
    }

    public void setBufferId(String bufferId) {
        this.bufferId = bufferId;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

}
