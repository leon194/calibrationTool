package com.bellus3d.android.arc.b3d4client;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.PowerManager;

import com.bellus3d.android.arc.b3d4client.ConfigService.CameraConfigService;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.io.File;
import java.math.BigInteger;
import java.net.NetworkInterface;
import java.security.MessageDigest;
import java.util.Collections;
import java.util.List;
import java.util.regex.Pattern;

public class GlobalResourceService {
    public static final String TAG = "ArcClient";

    public static final String STRING_BUFFER_CAPTURE = "buffer_capture";
    public static final String STRING_BUFFER_MERGE  = "buffer_merge";
    public static final String STRING_BUFFER_FETCH  = "buffer_fetch";
    public static final String STRING_STREAM_START   = "stream_start";
    public static final String STRING_STREAM_STOP    = "buffer_stop";
    public static final String STRING_STREAM_CAPTURE = "stream_capture";
    public static final String STRING_STREAM_DEPTH = "stream_depth";
    public static final String STRING_BUFFER_COLOR     = "buffer_color";
    public static final String STRING_BUFFER_RELEASE = "buffer_release";
    public static final String STRING_BUFFER_CANCEL   = "buffer_cancel";
    public static final String STRING_BUFFER_SNAPSHOT = "buffer_snapshot";
    public static final String STRING_BUFFER_SNAPSHOT_RECEIVE = "buffer_snapshot_receive";
    public static final String STRING_RECALIBRATION   = "recalibration";
    public static final String STRING_CANCEL_PROCESS   = "process_cancel";

    public static final String STRING_BUFFER_COLOR_ARC1   = "arc1";
    public static final String STRING_BUFFER_COLOR_ARCX   = "arcx_fullhead";

    public static final String STRING_STREAM_MODE_NORMAL          = "normal";
    public static final String STRING_STREAM_MODE_SELFSCAN         = "selfscan";
    public static final String STRING_STREAM_MODE_OPERATORSCAN   = "operatorscan";

    public static final String FRAME_TYPE_COLOR = "color";
    public static final String FRAME_TYPE_IRL = "IRL";
    public static final String FRAME_TYPE_IRR = "IRR";
    public static final String FRAME_TYPE_STEREO = "stereos";
    public static final String FRAME_TYPE_DEPTH = "depth";
    public static final String FRAME_TYPE_CALIZIP = "calibrationzip";
    public static final String FRAME_TYPE_FACELM = "facelandmark";
    public static final String FRAME_TYPE_COLOR_NOTIFICATION = "color_timestamp";

    public static final String STRING_TRUE = "true";
    public static final String STRING_FALSE = "false";

    public static final String REQUEST_TYPE_SNAPSHOT = "snapshot";
    public static final String REQUEST_TYPE_PREVIEW = "preview";

    public static final String RECALIBRATION_AUTO = "AUTO";
    public static final String RECALIBRATION_MANUAL = "MANUAL";

    public static final String BUFFER_CAPTURE_STILL = "BCStill";
    public static final String BUFFER_CAPTURE_4D    = "BC4D";

    public static final String BUFFER_CAPTURE_DIFF    = "buffer_capture_diff";

    public static final String ARC_APP_PATH = "/sdcard/Bellus3d/Arc/";
    public static final String ARC_CLIENT_PATH = "/sdcard/Bellus3d/Arc/ArcClient/";
    public static final String ARC_TEMP_PATH = "/sdcard/Bellus3d/Arc/ArcClient/sessions/temp/";
    public static final String ARC_LOG_PATH = "/sdcard/Bellus3d/Arc/ArcClient/logs/";

    public static final String FACTORY_ROOT_FOLDER_PATH = "/mnt/vendor/";
    public static final String FACTORY_CALIBRATION_FOLDER_PATH = FACTORY_ROOT_FOLDER_PATH + "CalibrationFiles";
    public static final String CALIBRATION_FOLDER_PATH = ARC_CLIENT_PATH + "CalibrationFiles";
    public static final String STASM410_FOLDER_PATH = ARC_CLIENT_PATH + "stasm4.1.0";
    public static final String DIAGNOSTIC_FOLDER_PATH = ARC_CLIENT_PATH + "diagnostic";
    public static final String SESSIONS_FOLDER_PATH = ARC_CLIENT_PATH + "sessions";
    public static final String LOGS_FOLDER_PATH = ARC_CLIENT_PATH + "logs";
    public static final String CONFIGS_FOLDER_PATH = ARC_CLIENT_PATH + "configs";
    public static final String DEBUG_FOLDER_PATH = ARC_CLIENT_PATH + "/debug";
    public static final String BUFFER_CAPTURE_DEBUG_FOLDER_PATH = DEBUG_FOLDER_PATH + "/buffer_capture";
    public static final String RECALIBRATION_FOLDER_PATH = DEBUG_FOLDER_PATH + "/Recalibration";
    public static final String ARC1_FOLDER_PATH = ARC_CLIENT_PATH + "Arc1";
    public static final String ARC1_FACE_LANDMARk_FOLDER_PATH = ARC1_FOLDER_PATH + "/FaceLandMark";
    public static final String ARC1_FACE_LANDMARk_FILE = ARC1_FACE_LANDMARk_FOLDER_PATH + "/facelandmark.yml";

    public static final String B3DCALIBDATA_FILE_PATH = ARC_CLIENT_PATH + "CalibrationFiles/b3dCalibData.bin";
    public static final String LEFTCAM_FILE_PATH = ARC_CLIENT_PATH + "CalibrationFiles/leftCam.yml";
    public static final String MIDCAM_FILE_PATH = ARC_CLIENT_PATH + "CalibrationFiles/midCam.yml";
    public static final String RIGHTCAM_FILE_PATH = ARC_CLIENT_PATH + "CalibrationFiles/rightCam.yml";
    public static final String DEPTHCAM_FILE_PATH = ARC_CLIENT_PATH + "CalibrationFiles/depthCam.yml";
    public static final String FRONTALFACE_FILE_PATH = ARC_CLIENT_PATH + "stasm4.1.0/data/haarcascade_frontalface_alt2.xml";
    public static final String LEFTEYE_FILE_PATH = ARC_CLIENT_PATH + "stasm4.1.0/data/haarcascade_mcs_lefteye.xml";
    public static final String MOUTH_FILE_PATH = ARC_CLIENT_PATH + "stasm4.1.0/data/haarcascade_mcs_mouth.xml";
    public static final String RIGHTEYE_FILE_PATH = ARC_CLIENT_PATH + "stasm4.1.0/data/haarcascade_mcs_righteye.xml";
    public static final String CONFIG_FILE_PATH = "/Bellus3d/Arc/ArcClient/configs/config.json";
    public static final String DIAGNOSTIC_M_FILE_PATH = DIAGNOSTIC_FOLDER_PATH +  "/diagnosticM.jpg";
    public static final String DIAGNOSTIC_L_FILE_PATH = DIAGNOSTIC_FOLDER_PATH +  "/diagnosticL.png";
    public static final String DIAGNOSTIC_R_FILE_PATH = DIAGNOSTIC_FOLDER_PATH +  "/diagnosticR.png";
    public static final String ARC_MMAP_FOLDER_PATH = ARC_CLIENT_PATH + "mm";
    public static final String ARC_ONE_MMAP_FOLDER_PATH = ARC_MMAP_FOLDER_PATH + "/one";
    public static final String ARC_STILL_MMAP_FOLDER_PATH = ARC_MMAP_FOLDER_PATH + "/still";
    public static final String ARC_MOTION_MMAP_FOLDER_PATH = ARC_MMAP_FOLDER_PATH + "/motion";

    public static final String ARC_MANUFACTURER = "sprd";
    public static final String ARC_MODEL = "s9863a1h10_Natv";
    public static final String ARC_MODEL2 = "Bellus3D_Arc";
    public static final String WEB_SOCKET_PROTO = "ws";
    public static final String hsPass = "HappyB3D";
    public static final Pattern hsRegex = Pattern.compile("^B3D4_(.*)");
    public static final Pattern ipRegex = Pattern.compile("^192\\.168\\.137\\.");
    public static final int TIME_DIFFERENCE = 0;
    public static final boolean WRITE_CRASHES_TO_FILE = true; //Change if you don't want to write to file.
    public static boolean WRITE_TO_FILE = false;
    public static final boolean WRITE_TIMESTAMP = true;

    public static int ARC_2M_COLOR_WIDTH = 1224;
    public static int ARC_2M_COLOR_HEIGHT = 1632;
    public static int ARC_2M_IR_WIDTH = 800;
    public static int ARC_2M_IR_HEIGHT = 1280;
    public static int ARC_2M_COLOR_SIZE = (int) (ARC_2M_COLOR_WIDTH * ARC_2M_COLOR_HEIGHT * 1.5);
    public static int ARC_2M_IR_SIZE = (ARC_2M_IR_WIDTH * ARC_2M_IR_HEIGHT);
    public static int ARC_2M_TIMESTAMP_SIZE = 8;
    public static int ARC_2M_TIMESTAMP_COLOR_START = ARC_2M_COLOR_SIZE + ARC_2M_IR_SIZE * 2 ;
    public static int ARC_2M_TIMESTAMP_IRL_START = ARC_2M_TIMESTAMP_COLOR_START + ARC_2M_TIMESTAMP_SIZE;
    public static int ARC_2M_TIMESTAMP_IRR_START = ARC_2M_TIMESTAMP_IRL_START + ARC_2M_TIMESTAMP_SIZE;
    public static int ARC_2M_TIMESTAMP_IRR_END = ARC_2M_TIMESTAMP_IRR_START + ARC_2M_TIMESTAMP_SIZE;
    public static long NSTOMS = 1000000;
    public static long MSTOSEC = 1000;

    public static int EXP_LINE_TIME_2M = 13416;
    public static int EXP_LINE_TIME_8M = 26833;
    public static int OV8856_BINNING_FACTOR = 1;
    
    public static int ARC_8M_COLOR_WIDTH = 2448;
    public static int ARC_8M_COLOR_HEIGHT = 3264;
    public static int ARC_8M_IR_WIDTH = 800;
    public static int ARC_8M_IR_HEIGHT = 1280;
    public static int ARC_8M_COLOR_SIZE = (int) (ARC_8M_COLOR_WIDTH * ARC_8M_COLOR_HEIGHT * 1.5);
    public static int ARC_8M_IR_SIZE = (ARC_8M_IR_WIDTH * ARC_8M_IR_HEIGHT);
    public static int ARC_8M_TIMESTAMP_SIZE = 8;
    public static int ARC_8M_TIMESTAMP_COLOR_START = ARC_8M_COLOR_SIZE + ARC_8M_IR_SIZE * 2 ;
    public static int ARC_8M_TIMESTAMP_IRL_START = ARC_8M_TIMESTAMP_COLOR_START + ARC_8M_TIMESTAMP_SIZE;
    public static int ARC_8M_TIMESTAMP_IRR_START = ARC_8M_TIMESTAMP_IRL_START + ARC_8M_TIMESTAMP_SIZE ;
    public static int ARC_8M_TIMESTAMP_IRR_END = ARC_8M_TIMESTAMP_IRR_START + ARC_8M_TIMESTAMP_SIZE;

    public static int CURRENT_LOG_LEVEL = 2; // this will only control loglevel, not control save debug image or not

    public static int BUFFER_FETCH_TYPE_COLOR = 1<<0;
    public static int BUFFER_FETCH_TYPE_IRL   = 1<<1;
    public static int BUFFER_FETCH_TYPE_IRR   = 1<<2;
    public static int BUFFER_FETCH_TYPE_DEPTH = 1<<3;
    public static int BUFFER_FETCH_TYPE_CALIB = 1<<4;

    private static Context _context;

    public GlobalResourceService(Context context){
        _context = context;
    }

    public static enum CAMERAID {
        CAMERAID_RGB("0"),
        CAMERAID_IRR("1"),
        CAMERAID_IRL("2"),
        CAMERAID_MULTI("17");

        private final String id;

        CAMERAID(String id) { this.id = id; }

        public String getValue() { return id; }
    }

    public static boolean isARC(){
        if (!Build.MANUFACTURER.equals(ARC_MANUFACTURER)) return false;
        if (!(Build.MODEL.equals(ARC_MODEL) ||
                Build.MODEL.equals(ARC_MODEL2))) return false;
        return true;
    }

    public static String getDeviceManufacturer(){
        return Build.MANUFACTURER;
    }

    public static String getDeviceModel(){
        return Build.MODEL;
    }

    public static int getSDKVersion(){
        return android.os.Build.VERSION.SDK_INT;
    }


    public static int getAppBuildNumber(){
        try {
            return (_context.getPackageManager().getPackageInfo(_context.getPackageName(), 0)).versionCode;
        } catch (PackageManager.NameNotFoundException e) {
            LogService.e(TAG,e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
        return -1;
    }

    public static String getAppVersionName(){
        try {
            return (_context.getPackageManager().getPackageInfo(_context.getPackageName(), 0)).versionName;
        } catch (PackageManager.NameNotFoundException e) {
            LogService.e(TAG,e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
        return "";
    }

    public static String getDeviceId() {
        return getAdbId();
    }

    public static String getAdbId() {
        return CameraConfigService.get("ro.serialno","Unknown");
    }

    public static String getOSVersion() {
        return CameraConfigService.get("ro.build.display.id","Unknown");
    }

    private static String getWifiMacAddress() {
        try {
            String interfaceName = "wlan0";
            List<NetworkInterface> interfaces = Collections.list(NetworkInterface.getNetworkInterfaces());
            for (NetworkInterface intf : interfaces) {
                if (!intf.getName().equalsIgnoreCase(interfaceName)){
                    continue;
                }

                byte[] mac = intf.getHardwareAddress();
                if (mac==null){
                    return "";
                }

                StringBuilder buf = new StringBuilder();
                for (byte aMac : mac) {
                    buf.append(String.format("%02X:", aMac));
                }
                if (buf.length()>0) {
                    buf.deleteCharAt(buf.length() - 1);
                }
                return buf.toString();
            }
        } catch (Exception ignore) {}
        return "";
    }

    // Hash a string
    private static String getHash(String[] data, int len) {
        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");
            for (int i=0; i< data.length; i++) {
                digest.update(data[i].getBytes("UTF-8"));
            }
            byte[] hashBytes = digest.digest();
            BigInteger bi = new BigInteger(1, hashBytes);
            String hash = String.format("%0" + (hashBytes.length << 1) + "X", bi);
            if (len !=0) hash = hash.substring(0, len);
            return hash;
        } catch (Exception e) {
            LogService.e(TAG, "getCredential Digest error: "+e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return null;
        }
    }

    public static void restartClient() {
        PackageManager packageManager = _context.getPackageManager();
        Intent intent = packageManager.getLaunchIntentForPackage(_context.getPackageName());
        ComponentName componentName = intent.getComponent();
        Intent mainIntent = Intent.makeRestartActivityTask(componentName);
        mainIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_CLEAR_TASK);
        _context.startActivity(mainIntent);
        Runtime.getRuntime().exit(0);
    }

    public static void rebootDevice(String reason) {
        PowerManager pm = (PowerManager) _context.getSystemService(_context.POWER_SERVICE);
        pm.reboot(reason);
    }

    public static void killselfPid() {
        int pid = android.os.Process.myPid();
        String command = "kill -9 " + pid;
        LogService.i(TAG, "killselfPid : " + command);
        try {
            Runtime.getRuntime().exec(command);
            System.exit(0);
        } catch (Exception e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    public static void checkCalibrationFiles() {
        /* calibrationfiles */
        DiskService.createDirectory(new File(CALIBRATION_FOLDER_PATH));
        if(!(DiskService.fileExists(B3DCALIBDATA_FILE_PATH) && DiskService.fileExists(LEFTCAM_FILE_PATH) &&
                DiskService.fileExists(MIDCAM_FILE_PATH)  && DiskService.fileExists(RIGHTCAM_FILE_PATH)))
            DiskService.copyDirectory(FACTORY_CALIBRATION_FOLDER_PATH,ARC_CLIENT_PATH);

        /* copy leftCam.yml to depthCam.yml*/
        if(!DiskService.fileExists(DEPTHCAM_FILE_PATH))
            DiskService.copyFile(LEFTCAM_FILE_PATH,DEPTHCAM_FILE_PATH);
    }

    public static void checkSTasmFiles() {
        /* stasm4.1.0 */
        DiskService.createDirectory(new File(STASM410_FOLDER_PATH));
        if(!(DiskService.fileExists(FRONTALFACE_FILE_PATH) && DiskService.fileExists(MOUTH_FILE_PATH) &&
                DiskService.fileExists(LEFTEYE_FILE_PATH)  && DiskService.fileExists(RIGHTEYE_FILE_PATH)))
            DiskService.copyAssetFolder(_context,"stasm4.1.0",STASM410_FOLDER_PATH);
    }

    public static void checkdiagnosticFiles() {
        DiskService.createDirectory(new File(DIAGNOSTIC_FOLDER_PATH));
        if(!(DiskService.fileExists(DIAGNOSTIC_M_FILE_PATH) && DiskService.fileExists(DIAGNOSTIC_L_FILE_PATH) &&
                DiskService.fileExists(DIAGNOSTIC_R_FILE_PATH)))
            DiskService.copyAssetFolder(_context,"diagnostic",DIAGNOSTIC_FOLDER_PATH);
    }
}
