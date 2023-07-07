package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

import java.util.List;

public class Common {

    /**
     * request : hello
     * request_id : 20200107122348152
     * hostname : DESKTOP-OB50ECL
     * hostaddr : 192.168.137.1
     * version : 0.1.5
     */

    private String request;
    @SerializedName("request_id")
    private String requestId;
    private String hostname;
    @SerializedName("hostaddr")
    private String hostAddress;
    private String version;
    /**
     * status : OK
     * response_to : connect
     */

    private String status;
    @SerializedName("response_to")
    private String responseTo;

    private boolean debug;
    private String error;
    private String type;

    public String getRequest() {
        return request;
    }

    public void setRequest(String request) {
        this.request = request;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getHostname() {
        return hostname;
    }

    public void setHostname(String hostname) {
        this.hostname = hostname;
    }

    public String getHostAddress() {
        return hostAddress;
    }

    public void setHostAddress(String hostAddress) {
        this.hostAddress = hostAddress;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public String getResponseTo() {
        return responseTo;
    }

    public void setResponseTo(String responseTo) {
        this.responseTo = responseTo;
    }

    public boolean isDebug() {
        return debug;
    }

    public void setDebug(boolean debug) {
        this.debug = debug;
    }

    public String getError() {
        return error;
    }

    public void setError(String error) {
        this.error = error;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }
}
