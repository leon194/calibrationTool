package com.bellus3d.android.arc.b3d4client.ConfigService;

import android.annotation.SuppressLint;
import android.util.Size;

import com.bellus3d.android.arc.b3d4client.DiskService;
import com.bellus3d.android.arc.b3d4client.GlobalResourceService;
import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.DeviceConfig;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.google.gson.Gson;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class LocalConfigService {
    public static String currentLayoutPosition = "null";

    public LocalConfigService(){ }

    public void saveConfigFile(DeviceConfig deviceConfig){
        String configStr = new Gson().toJson(deviceConfig);
        DiskService.overwriteContentToFile(
                GlobalResourceService.CONFIGS_FOLDER_PATH,
                "config.json",
                configStr
                );
    }

    public DeviceConfig loadConfigFile(String path) {
        LogService.d(TAG, "loadConfigFile: " + path);
        String config = DiskService.sdcardLoadFile(path);
        Gson gson = new Gson();
        DeviceConfig deviceConfig = null;
        try {
            deviceConfig = gson.fromJson(config, DeviceConfig.class);
        } catch (Exception e) {
            LogService.e(TAG, e.getMessage());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }

        currentLayoutPosition = deviceConfig.getLayoutPosition();
        return deviceConfig;
    }

    private void _save() {
        LogService.d(TAG, "_save: "+toString());
        DiskService.sdcardSaveFile(GlobalResourceService.CONFIG_FILE_PATH, toString());
    }

    @SuppressLint("NewApi")
    public static Size parseDimension(String dimension) {
        if (dimension == null || dimension.isEmpty()) return new Size(0, 0);

        String[] values = dimension.toLowerCase().split("x", 2);
        int w = Integer.parseInt(values[0]);
        int h = Integer.parseInt(values[1]);
        return new Size(w, h);
    }

    public static native void setArcNativeRootPathJNI(String rootPath);

    public static native void setArc1LandMarkPathJNI(String rootPath);

    public static native int[] GetLastAEAWBValueJNI();

    public static native void updateSaveResultFlagJNI(boolean debug);
}
