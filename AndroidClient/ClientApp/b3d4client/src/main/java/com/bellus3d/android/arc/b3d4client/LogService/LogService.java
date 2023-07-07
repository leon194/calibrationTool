package com.bellus3d.android.arc.b3d4client.LogService;

import android.util.Log;

import com.bellus3d.android.arc.b3d4client.DiskService;
import com.bellus3d.android.arc.b3d4client.GlobalResourceService;
import com.bellus3d.android.arc.b3d4client.TimeStampService;
import com.example.b3d4newclient.BuildConfig;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.ARC_LOG_PATH;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.WRITE_TO_FILE;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.*;

public class LogService {
    public enum Level {
        TRACE(0),
        DEBUG(1),
        INFO(2),
        WARN(3),
        ERROR(4),
        WTF(5);

        private int index;

        Level(int index) {
            this.index = index;
        }

        public int getIndex() {
            return this.index;
        }

        public static Level fromInteger(int x) {
            switch(x) {
                case 0:
                    return TRACE;
                case 1:
                    return DEBUG;
                case 2:
                    return INFO;
                case 3:
                    return WARN;
                case 4:
                    return ERROR;
                case 5:
                    return WTF;
                default:
                    return INFO;
            }
        }
    };

    private enum NativeLevel {
        ALL(0),
        VERBOSE(1),
        INFO(2),
        WARNING(3),
        ERROR(4),
        ALWAYS(5),
        NONE(10);

        private int index;

        private NativeLevel(int index) {
            this.index = index;
        }

        public int getIndex() {
            return this.index;
        }
    };

    public static void setLogLevel(Level level) {CURRENT_LEVEL = level;};

    public static void v(String enable_tag, String message){
        if(CURRENT_LEVEL.getIndex() <= Level.TRACE.getIndex()) {
            String logs = getPidTidClassNameMethodNameAndLineNumber() + message;
            android.util.Log.v(enable_tag, logs);
            if(WRITE_TO_FILE) DiskService.appendToFile(_currentLogFolder, CURRENT_LOG_FILE, TimeStampService.getCurrentTimeStamp()
                    + " : " +logs + "\n");
        }
    }

    public static void d(String enable_tag, String message){
        if(CURRENT_LEVEL.getIndex() <= Level.DEBUG.getIndex()) {
            String logs = getPidTidClassNameMethodNameAndLineNumber() + message;
            android.util.Log.d(enable_tag, logs);
            if(WRITE_TO_FILE) DiskService.appendToFile(_currentLogFolder, CURRENT_LOG_FILE, TimeStampService.getCurrentTimeStamp()
                    + " : " +logs + "\n");
        }
    }

    public static void i(String enable_tag, String message){
        if(CURRENT_LEVEL.getIndex() <= Level.INFO.getIndex()) {
            String logs = getPidTidClassNameMethodNameAndLineNumber() + message;
            android.util.Log.i(enable_tag, logs);
            if(WRITE_TO_FILE) DiskService.appendToFile(_currentLogFolder, CURRENT_LOG_FILE, TimeStampService.getCurrentTimeStamp()
                    + " : " +logs + "\n");
        }
    }

    public static void w(String enable_tag, String message){
        if(CURRENT_LEVEL.getIndex() <= Level.WARN.getIndex()) {
            String logs = getPidTidClassNameMethodNameAndLineNumber() + message;
            android.util.Log.w(enable_tag, logs);
            if(WRITE_TO_FILE) DiskService.appendToFile(_currentLogFolder, CURRENT_LOG_FILE, TimeStampService.getCurrentTimeStamp()
                    + " : " +logs + "\n");
        }
    }

    public static void e(String enable_tag, String message){
        if(CURRENT_LEVEL.getIndex() <= Level.ERROR.getIndex()) {
            String logs = getPidTidClassNameMethodNameAndLineNumber() + message;
            android.util.Log.e(enable_tag, logs);
            if(WRITE_TO_FILE) DiskService.appendToFile(_currentLogFolder, CURRENT_LOG_FILE, TimeStampService.getCurrentTimeStamp()
                    + " : " +logs + "\n");
        }
    }

    public static void wtf(String enable_tag, String message,Object...args){
        if(CURRENT_LEVEL.getIndex() <= Level.WTF.getIndex()) {
            String logs = getPidTidClassNameMethodNameAndLineNumber() + message + args;
            android.util.Log.wtf(enable_tag, logs);
            if(WRITE_TO_FILE) DiskService.appendToFile(_currentLogFolder, CURRENT_LOG_FILE, TimeStampService.getCurrentTimeStamp()
                    + " : " +logs + "\n");
        }
    }

    public static void logStackTrace(String enable_tag, StackTraceElement[] stes) {
        StringBuilder sb = new StringBuilder();
        sb.append(System.getProperties().getProperty("line.separator"));
        for (StackTraceElement ste : stes) {
            sb.append(ste.toString());
            sb.append(System.getProperties().getProperty("line.separator"));
        }
        e(enable_tag, sb.toString());
    }

    public static int getProcessID() {
        return android.os.Process.myPid();
    }

    public static long getThreadId() {
        return android.os.Process.myTid();
    }

    private static final int STACK_TRACE_LEVELS_UP = 5;

    public static int getLineNumber()
    {
        return Thread.currentThread().getStackTrace()[STACK_TRACE_LEVELS_UP].getLineNumber();
    }

    public static String getClassName()
    {
        String fileName = Thread.currentThread().getStackTrace()[STACK_TRACE_LEVELS_UP].getFileName();

        return fileName.substring(0, fileName.length() - 5);
    }

    public static String getMethodName()
    {
        return Thread.currentThread().getStackTrace()[STACK_TRACE_LEVELS_UP].getMethodName();
    }

    public static String getClassNameAndMethodName()
    {
        return "[" + getClassName() + "." + getMethodName() + "() " + "]: ";
    }


    public static String getClassNameMethodNameAndLineNumber()
    {
        return "[" + getClassName() + "." + getMethodName() + "() -" + getLineNumber() + "]: ";
    }

    public static String getPidTidClassNameMethodNameAndLineNumber()
    {
        return " [" + getProcessID() + "-" + getThreadId() + "] " +
                " [" + getClassName() + "." + getMethodName() + "() -" + getLineNumber() + "]: ";
    }

    private static Level CURRENT_LEVEL = BuildConfig.DEBUG ? Level.TRACE : Level.WARN;

    private static String _currentLogFolder = ARC_LOG_PATH;

    private static String CRASH_LOG_FILE = "AndroidCrash.log";
    private static String CURRENT_LOG_FILE = "ArcClient.log";
    private static String initMessage = "";

    private static boolean isEnabled(Level l) { return l.getIndex() >= CURRENT_LEVEL.getIndex(); }

    /**
     * Only log to file when there is crash
     *
     * @param message
     */
    public static void logCrash(String message) {
        if (GlobalResourceService.WRITE_CRASHES_TO_FILE) {
            String callingClass = getCallerTag();
            Log.d(callingClass, message);
            if(!DiskService.fileExists(_currentLogFolder + "/" + CRASH_LOG_FILE))
                DiskService.appendToFile(_currentLogFolder, CRASH_LOG_FILE, initMessage + "\n");
            DiskService.appendToFile(_currentLogFolder, CRASH_LOG_FILE, message);
        }
    }

    /**
     * Set this in MainActivity oncreate to make sure that there is only one log file created for
     * each run of the app
     * @param timestamp a string represent the time stamp when created MainActivity
     */
    public static void initLogFile(String timestamp){
        _currentLogFolder = _currentLogFolder + "/" + timestamp;
        CRASH_LOG_FILE = timestamp+"_"+CRASH_LOG_FILE;
        CURRENT_LOG_FILE = timestamp+"_"+CURRENT_LOG_FILE;
        initMessage =
                "=============================================="     + "\n" +
                "Device Model : " + getDeviceModel() + "\n" +
                "SDK Version : " + getSDKVersion()  + "\n" +
                "App Build Number : " + getAppBuildNumber() + "\n" +
                "App Version Number : " + getAppVersionName() + "\n" +
                "Adb ID : " + getAdbId() + "\n" +
                "=============================================="   + "\n";
        if(WRITE_TO_FILE)
            DiskService.appendToFile(_currentLogFolder, CURRENT_LOG_FILE, initMessage + "\n");
    }

    public static void initNativeLogFile(){

        /* *
        *       JAVA                       NATIVE
        *       TRACE (LogService.v)       LOG_ALL      (logAll)
        *       DEBUG (LogService.d)       LOG_VERBOSE  (logVerbose)
        *       INFO  (LogService.i)       LOG_INFO     (logInfo)
        *       WARN  (LogService.w)       LOG_WARNING  (logWarning)
        *       ERROR (LogService.e)       LOG_ERROR    (logError)
        *       WTF   (LogService.wtf)     LOG_ALWAYS   (logAlways)
        * */
        setLogPathJNI(_currentLogFolder + "/" + CURRENT_LOG_FILE,CURRENT_LOG_LEVEL);
    }

    // returns calling function and file name to be used as tag for log messages
    private static String getCallerTag() {
        StackTraceElement[] stElements = Thread.currentThread().getStackTrace();
        for (int i = 1; i < stElements.length; i++) {
            StackTraceElement ste = stElements[i];
            if (!ste.getClassName().equals(LogService.class.getName()) && ste.getClassName().indexOf("java.lang.Thread") != 0) {
                String fileName = ste.getFileName();
                fileName = fileName.replace(".java", "");
                if (GlobalResourceService.WRITE_TIMESTAMP) {
                    return fileName + "|" + ste.getLineNumber() + "|" + ste.getMethodName() + "|" + TimeStampService.getCurrentTimeStamp() + "|";
                } else {
                    return fileName + "|" + ste.getLineNumber() + "|" + ste.getMethodName() + "|";
                }
            }
        }
        return null;
    }

    private static native void setLogPathJNI(String path, int nativeLogLevel);
}
