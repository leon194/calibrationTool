package com.bellus3d.android.arc.b3d4client.JsonModel;

import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.DeviceConfig;

public class DeviceConfigResponseTo extends ResponseTo {

    private DeviceConfig configuration;

    public DeviceConfig getConfiguration() {
        return configuration;
    }

    public void setConfiguration(DeviceConfig configuration) {
        this.configuration = configuration;
    }
}
