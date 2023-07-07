package com.bellus3d.android.arc.b3d4client.ConfigService;

import android.text.TextUtils;

import com.bellus3d.android.arc.b3d4client.DiskService;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.lang.reflect.Method;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class CameraConfigService {

    private static Class<?> mClassType = null;
    private  static  Method mSetMethod =  null;
    private static Method mGetMethod = null;
    private static Method mGetIntMethod = null;
    private static Method mGetBooleanMethod = null;

    public static void setCamera17FPS (int fps, long exptime) {

        String exposuretime, RGBFPS;
        String ExpRegValue = null, StrobeShiftRegValueReverse = null ;

        long StrobeShiftRegValue = 0;
        int frame_length_line, Exposure_time_line;
        float Line_period_us = 18.2f;
        String hex = null , hexswap = null, rstFSIN = null ; ;

        switch(fps) {
            case 5 : {
                exposuretime = "200000";
                RGBFPS = "0505";
                frame_length_line = 10945;
                Exposure_time_line = (int)(exptime / Line_period_us);
                ExpRegValue = Long.toString(Exposure_time_line * 16);
                StrobeShiftRegValue = (long) (frame_length_line-Exposure_time_line-7-(1100/Line_period_us));
                hex = Long.toHexString(StrobeShiftRegValue);
                switch(hex.length()) {
                    case 3 : { hex = "0" + hex;   }  break;
                    case 2 : { hex = "00" + hex;  }  break;
                    case 1 : { hex = "000" + hex; }  break;
                }
                hexswap = DiskService.swap(hex,0,2);
                hexswap = DiskService.swap(hexswap,1,3);
                StrobeShiftRegValueReverse = Long.toString(Long.parseLong(hexswap, 16));
                rstFSIN = "28714"; //2a70
            }
            break;
            case 10 : {
                exposuretime = "100000";
                RGBFPS = "1010";
                frame_length_line = 5485;
                Exposure_time_line = (int)(exptime / Line_period_us);
                ExpRegValue = Long.toString(Exposure_time_line * 16);
                StrobeShiftRegValue = (long) (frame_length_line-Exposure_time_line-7-(1100/Line_period_us));
                hex = Long.toHexString(StrobeShiftRegValue);
                switch(hex.length()) {
                    case 3 : { hex = "0" + hex;   }  break;
                    case 2 : { hex = "00" + hex;  }  break;
                    case 1 : { hex = "000" + hex; }  break;
                }
                hexswap = DiskService.swap(hex,0,2);
                hexswap = DiskService.swap(hexswap,1,3);
                StrobeShiftRegValueReverse = Long.toString(Long.parseLong(hexswap, 16));
                rstFSIN = "45066"; //0AB0
            }
            break;
            case 15 : {
                exposuretime = "66666";
                RGBFPS = "1515";
                frame_length_line = 3664; // 3639+25
                Exposure_time_line = (int)(exptime / Line_period_us);
                ExpRegValue = Long.toString(Exposure_time_line * 16);
                StrobeShiftRegValue = (long) (frame_length_line-Exposure_time_line-7-(1100/Line_period_us));
                hex = Long.toHexString(StrobeShiftRegValue);
                switch(hex.length()) {
                    case 3 : { hex = "0" + hex;   }  break;
                    case 2 : { hex = "00" + hex;  }  break;
                    case 1 : { hex = "000" + hex; }  break;
                }
                hexswap = DiskService.swap(hex,0,2);
                hexswap = DiskService.swap(hexswap,1,3);
                StrobeShiftRegValueReverse = Long.toString(Long.parseLong(hexswap, 16));
                rstFSIN = "45066"; //0AB0
            }
            break;
            case 20 : {
                exposuretime = "50000";
                RGBFPS = "2020";
                frame_length_line = 2755;
                Exposure_time_line = (int)(exptime / Line_period_us);
                ExpRegValue = Long.toString(Exposure_time_line * 16);
                StrobeShiftRegValue = (long) (frame_length_line-Exposure_time_line-7-(1100/Line_period_us));
                hex = Long.toHexString(StrobeShiftRegValue);
                switch(hex.length()) {
                    case 3 : { hex = "0" + hex;   }  break;
                    case 2 : { hex = "00" + hex;  }  break;
                    case 1 : { hex = "000" + hex; }  break;
                }
                hexswap = DiskService.swap(hex,0,2);
                hexswap = DiskService.swap(hexswap,1,3);
                StrobeShiftRegValueReverse = Long.toString(Long.parseLong(hexswap, 16));
                rstFSIN = "45066"; //0AB0
            }
            break;
            case 30 : {
                exposuretime = "33000";
                RGBFPS = "3030";
            }
            break;
            default: {
                exposuretime = "100000";
                RGBFPS = "1010";
                ExpRegValue = StrobeShiftRegValueReverse = "0";
            }
            break;
        }

        LogService.d(TAG,"SetFPS (Color,IR) : (" + RGBFPS + ", " + exposuretime + ")");
        LogService.d(TAG,"SetExp (Value,Reg) : (" + exptime + "us, " + ExpRegValue + ")");
        LogService.d(TAG,"SetStrobeShift (Original,Reverse) : (" + hex + " ," + hexswap + ")");
        LogService.d(TAG,"SetStrobeShift (Original,Reverse) : (" + StrobeShiftRegValue + " ," + StrobeShiftRegValueReverse + ")");
        LogService.d(TAG,"SetrstFSIN  : (" + rstFSIN + ")");
        CameraConfigService.set("persist.camera.ae.ir2.expos", exposuretime);
        CameraConfigService.set("persist.camera.ae.ir1.expos", exposuretime);
        CameraConfigService.set("persist.vendor.cam.preview.fps", RGBFPS);
        CameraConfigService.set("persist.ov9282dual.shutter.0x3500-0x3502", ExpRegValue);
        CameraConfigService.set("persist.ov9282.shutter.0x3500-0x3502", ExpRegValue);
        CameraConfigService.set("persist.ov9282dual.strobe.0x392a-0x3929", StrobeShiftRegValueReverse);
        CameraConfigService.set("persist.ov9282dual.0x3827-0x3826", rstFSIN);
        CameraConfigService.set("persist.ov9282.0x3827-0x3826", rstFSIN);
    }

    public static void resetExpWhiteBalenceValue () {
        CameraConfigService.set("persist.camera.ov8856.shutter", String.valueOf(0));
        CameraConfigService.set("persist.camera.ov8856.gain", String.valueOf(0));
        CameraConfigService.set("persist.vendor.camera.bypass.ae", String.valueOf(0));
        CameraConfigService.set("persist.vendor.camera.bypass.awb", String.valueOf(0));
        CameraConfigService.set("debug.isp.awb.rgain.set", String.valueOf(0));
        CameraConfigService.set("debug.isp.awb.ggain.set", String.valueOf(0));
        CameraConfigService.set("debug.isp.awb.bgain.set", String.valueOf(0));
    }

    private static Class<?> getSystemPropertiesClass() throws ClassNotFoundException {
        if (mClassType == null) {
            mClassType = Class.forName("android.os.SystemProperties");
        }
        return mClassType;
    }

    private static Method getMethod() throws Exception {
        if (mGetMethod == null) {
            Class clazz = getSystemPropertiesClass();
            mGetMethod = clazz.getDeclaredMethod("get", String.class);
        }
        return mGetMethod;
    }

    private static Method getIntMethod() throws Exception {
        if (mGetIntMethod == null) {
            Class clazz = getSystemPropertiesClass();
            mGetIntMethod = clazz.getDeclaredMethod("getInt", String.class, int.class);
        }
        return mGetIntMethod;
    }

    private static Method getBooleanMethod() throws Exception {
        if (mGetBooleanMethod == null) {
            Class clazz = getSystemPropertiesClass();
            mGetBooleanMethod = clazz.getDeclaredMethod("getBoolean", String.class, boolean.class);
        }
        return mGetBooleanMethod;
    }

    public static String get(String key, String def) {
        try {
            String value = (String) getMethod().invoke(null, key);
            if (!TextUtils.isEmpty(value)) {
                return value;
            }
        } catch (Exception e) {
            LogService.e(TAG, "Unable to read system properties");
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }

        return def;
    }

    public static int getInt(String key, int def) {
        int value = def;
        try {
            value = (int) getIntMethod().invoke(null, key, def);
        } catch (Exception e) {
            LogService.e(TAG, "Unable to read system properties");
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
        return value;
    }

    public static boolean getBoolean(String key, boolean def) {
        boolean value = def;
        try {
            value = (Boolean) getBooleanMethod().invoke(null, key, def);
        } catch (Exception e) {
            LogService.e(TAG, "Unable to read system properties");
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
        return value;
    }
    private static Method setMethod()  {
        if (mSetMethod ==  null) {
            Class  clazz  = null;
            try {
                clazz = getSystemPropertiesClass();
                mSetMethod =  clazz.getDeclaredMethod("set", String.class, String.class);
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        }

        return mSetMethod;
    }

    public static void set(String  key, String  val)  {
        try {
            setMethod().invoke(null, key, val);
        } catch (Exception  e) {
            e.printStackTrace();
            LogService.e(TAG, "Unable to set system properties");
        }
    }
}