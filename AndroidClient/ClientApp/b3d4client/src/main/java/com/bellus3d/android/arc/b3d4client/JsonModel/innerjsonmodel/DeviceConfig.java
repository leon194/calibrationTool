package com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel;

import com.google.gson.annotations.SerializedName;

public class DeviceConfig {

    /**
     * hotspot : B3D4_Anyone
     * password : HappyB3D
     * hostaddr : 127.0.0.1
     * udpport : 3000
     * httpport : 3001
     * wsport : 3002
     * layoutPosition : l1
     * localConnection : true
     */

    private String hotspot;
    private String password;
    @SerializedName("hostaddr")
    private String hostAddress;
    @SerializedName("udpport")
    private int udpPort;
    @SerializedName("httpport")
    private int httpPort;
    @SerializedName("wsport")
    private int wsPort;
    private String layoutPosition;
    private Boolean localConnection;
    /**
     * layoutName : single
     * layoutDevices : 1
     */

    private String layoutName;
    private int layoutDevices;

    public String getHotspot() {
        return hotspot;
    }

    public void setHotspot(String hotspot) {
        this.hotspot = hotspot;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public String getHostAddress() {
        return hostAddress;
    }

    public void setHostAddress(String hostAddress) {
        this.hostAddress = hostAddress;
    }

    public int getUdpPort() {
        return udpPort;
    }

    public void setUdpPort(int udpPort) {
        this.udpPort = udpPort;
    }

    public int getHttpPort() {
        return httpPort;
    }

    public void setHttpPort(int httpPort) {
        this.httpPort = httpPort;
    }

    public int getWsPort() {
        return wsPort;
    }

    public void setWsPort(int wsPort) {
        this.wsPort = wsPort;
    }

    public String getLayoutPosition() {
        return layoutPosition;
    }

    public void setLayoutPosition(String layoutPosition) {
        this.layoutPosition = layoutPosition;
    }

    public Boolean getLocalConnection() {
        return localConnection;
    }

    public void setLocalConnection(Boolean localConnection) {
        this.localConnection = localConnection;
    }

    public String getLayoutName() {
        return layoutName;
    }

    public void setLayoutName(String layoutName) {
        this.layoutName = layoutName;
    }

    public int getLayoutDevices() {
        return layoutDevices;
    }

    public void setLayoutDevices(int layoutDevices) {
        this.layoutDevices = layoutDevices;
    }
}
