package com.bellus3d.android.arc.b3d4client;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Jia Liu on 3/16/2017.
 */

public class PermissionsService {

    private static final int REQUEST_CODE_B3D4_PERMISSION = 1300;
    Activity activity;

    public PermissionsService(final Activity activity){
        this.activity = activity;
    }

    String[] permissions = {
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.CAMERA
    };

    public boolean checkAllPermissionsEnabled() {
        for(String permission : permissions){
            if(ContextCompat.checkSelfPermission(activity, permission) != PackageManager.PERMISSION_GRANTED)
                return false;
        }
        return true;
    }

    public void requestMultiplePermissions(){
        List<String> remainingPermissions = new ArrayList<>();
        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(activity, permission) != PackageManager.PERMISSION_GRANTED) {
                remainingPermissions.add(permission);
            }
        }
        ActivityCompat.requestPermissions(activity, remainingPermissions.toArray(
                new String[remainingPermissions.size()]), REQUEST_CODE_B3D4_PERMISSION);
    }

    ArrayList<String> requestedPermissions = new ArrayList<String>();
    boolean writePermissionGranted = false;
    boolean readPermissionGranted = false;
    boolean cameraPermissionGranted = false;

    public boolean getAllPermissionGranted() {return writePermissionGranted && readPermissionGranted && cameraPermissionGranted;}

    public boolean getWritePermissionGranted() {return writePermissionGranted;}

    /**
     * Check if given permission was granted
     * @param permission Given Permission to be checked
     * @return true if permission already granted, false if not granted
     */
    public boolean checkPermission(String permission){
        int permissionCheck = ContextCompat.checkSelfPermission(activity, permission);
        if (permissionCheck == PackageManager.PERMISSION_DENIED) {
            requestedPermissions.add(permission);
            return false;
        } else if(permissionCheck == PackageManager.PERMISSION_GRANTED) {
            return true;
        }
        return false;
    }

    /**
     * With a list of not-yet-granted permission, request for these requestedPermissions
     */
    public void requestPermissions(){
        if (requestedPermissions.size() > 0) {
            ActivityCompat.requestPermissions(activity, requestedPermissions.toArray(new String[0]),
                    REQUEST_CODE_B3D4_PERMISSION);
        }
    }

    public void requestPermissionsResultAction(int requestCode,
                                               String permissions[], int[] grantResults){
        requestedPermissions.clear();
        switch (requestCode) {
            case PermissionsService.REQUEST_CODE_B3D4_PERMISSION: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0) {
                    for(int i = 0; i < permissions.length; i++){
                        if(permissions[i].equals("android.permission.WRITE_EXTERNAL_STORAGE")){
                            if(grantResults[i] != PackageManager.PERMISSION_GRANTED){
                                writePermissionGranted = false;
                            } else {
                                writePermissionGranted = true;
                            }
                        }
                        else if(permissions[i].equals("android.permission.CAMERA")){
                            if(grantResults[i] != PackageManager.PERMISSION_GRANTED){
                                cameraPermissionGranted = false;
                            } else {
                                cameraPermissionGranted = true;
                            }
                        }
                        else if(permissions[i].equals("android.permission.READ_EXTERNAL_STORAGE")){
                            if(grantResults[i] != PackageManager.PERMISSION_GRANTED){
                                readPermissionGranted = false;
                            } else {
                                readPermissionGranted = true;
                            }
                        }
                    }
                }
                return;
            }
        }
    }

    public void checkAndRequestPermissions(PermissionsGrantedCallback permissionsGrantedCallback){
        if(!checkAllPermissionsEnabled()){
            requestMultiplePermissions();
        } else {
            permissionsGrantedCallback.onPermissionsGranted();
        }
    }

    public interface PermissionsGrantedCallback{
        void onPermissionsGranted();
    }
}
