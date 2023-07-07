package com.bellus3d.android.arc.b3d4app;

import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.text.TextUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

public class CheckUtil {

    public static boolean isServiceWorked(Context context, String serviceName) {
        ActivityManager myManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        ArrayList<ActivityManager.RunningServiceInfo> runningService = (ArrayList<ActivityManager.RunningServiceInfo>) myManager.getRunningServices(Integer.MAX_VALUE);
        for (int i = 0; i < runningService.size(); i++) {
            if (runningService.get(i).service.getClassName().toString().equals(serviceName)) {
                return true;
            }
        }
        return false;
    }

    public static boolean isForeground(Context context, String PackageName) {
        ActivityManager myManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> task = myManager.getRunningTasks(1);
        ComponentName componentInfo = task.get(0).topActivity;
        if (componentInfo.getPackageName().equals(PackageName))
            return true;
        return false;
    }

    public static boolean isRunningProcess(Context context, String appInfo) {
        ActivityManager myManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> runningAppPs = myManager.getRunningAppProcesses();
        if (runningAppPs != null && runningAppPs.size() > 0) {
            if (runningAppPs.contains(appInfo)) {
                return true;
            }
        }
        return false;
    }

    public static boolean isClsRunning(Context context, String pkg, String cls) {
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> tasks = am.getRunningTasks(1);
        ActivityManager.RunningTaskInfo task = tasks.get(0);
        if (task != null) {
            return TextUtils.equals(task.topActivity.getPackageName(), pkg) &&
                    TextUtils.equals(task.topActivity.getClassName(), cls);
        }
        return false;
    }

    public static boolean isPkgRunning(Context context, String pkg) {
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> tasks = am.getRunningTasks(1);
        ActivityManager.RunningTaskInfo task = tasks.get(0);
        if (task != null) {
            return TextUtils.equals(task.topActivity.getPackageName(), pkg);
        }
        return false;
    }

    /* return null there is no files inside the directory */
    /* sort with decrease order */
    public static File[] sortFolderName(File directory) {
        File[] files = directory.listFiles();
        if (files != null) {
            if(files.length == 0 ) return null;
            Arrays.sort(files, new Comparator<File>() {
                public int compare(File o1, File o2) {
                    return o2.getName().compareTo(o1.getName()); //newest 1st
                }
            });
        }
        return files;
    }

    /* return null there is no files inside the directory */
    public static File[] sortFolderLastModifiedTime(File directory) {
        File[] files = directory.listFiles();
        if ( files != null) {
            if(files.length == 0 ) return null;
            Arrays.sort(files, new Comparator<File>() {
                public int compare(File o1, File o2) {
                    return String.valueOf(o2.lastModified()).compareTo(String.valueOf(o2.lastModified())); //newest 1st
                }
            });
        }
        return files;
    }
}
