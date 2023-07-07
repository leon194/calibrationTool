package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.google.gson.annotations.SerializedName;

public class HelloRequest {

    /**
     * request : hello
     * request_id : 20120101031734912
     * adb_id : BL1909070078
     * build_no : 819
     * rssi : -50
     * device_id : DB7C76D2F523F180
     * device_model : Bellus3D_Arc
     * manufacturer : sprd
     * client_ip : 192.168.137.114
     * version : null:henry_websocket_singleton:e078310
     */

    private String request;
    @SerializedName("request_id")
    private String requestId;
    @SerializedName("adbId")
    private String adbId;
    @SerializedName("build_no")
    private String buildNo;
    private String rssi;
    @SerializedName("device_id")
    private String deviceId;
    @SerializedName("device_model")
    private String deviceModel;
    private String manufacturer;
    @SerializedName("client_ip")
    private String clientIp;
    private String version;

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

    public String getAdbId() {
        return adbId;
    }

    public void setAdbId(String adbId) {
        this.adbId = adbId;
    }

    public String getBuildNo() {
        return buildNo;
    }

    public void setBuildNo(String buildNo) {
        this.buildNo = buildNo;
    }

    public String getRssi() {
        return rssi;
    }

    public void setRssi(String rssi) {
        this.rssi = rssi;
    }

    public String getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(String deviceId) {
        this.deviceId = deviceId;
    }

    public String getDeviceModel() {
        return deviceModel;
    }

    public void setDeviceModel(String deviceModel) {
        this.deviceModel = deviceModel;
    }

    public String getManufacturer() {
        return manufacturer;
    }

    public void setManufacturer(String manufacturer) {
        this.manufacturer = manufacturer;
    }

    public String getClientIp() {
        return clientIp;
    }

    public void setClientIp(String clientIp) {
        this.clientIp = clientIp;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }
}
