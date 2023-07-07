package com.bellus3d.android.b3d4.ratesttool;

import android.content.pm.PackageManager;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by alexp on 1/26/2018.
 */

public class PermissionService {

    public static final int REQUEST_CODE_B3D4_PERMISSION = 1300;

    private RATestTool activity;

    String[] permissions;
	
     PermissionService(final RATestTool activity, final String[] permissions) {
        this.activity = activity;
        this.permissions = permissions;
    }

    public boolean checkAllPermissionsEnabled() {
        for(String permission : permissions){
            if(activity.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED)
                return false;
        }
        return true;
    }

    public void requestMultiplePermissions(){
        List<String> remainingPermissions = new ArrayList<>();
        for (String permission : permissions) {
            if (activity.checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                remainingPermissions.add(permission);
            }
        }
        activity.requestPermissions(remainingPermissions.toArray(new String[remainingPermissions.size()]), REQUEST_CODE_B3D4_PERMISSION);
    }
}