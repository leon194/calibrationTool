package com.bellus3d.android.arc.b3d4client.JsonModel;

public class PongRequest extends Request {

    /**
     * rssi : null
     * type : "keepalive"
     */

    private String rssi;
    private String type;

    public String getRssi() {
        return rssi;
    }

    public void setRssi(String rssi) {
        this.rssi = rssi;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }
}
