package com.bellus3d.android.arc.b3d4app;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.icu.text.DateFormat;
import android.icu.text.SimpleDateFormat;
import android.os.Build;
import android.os.IBinder;
import android.support.annotation.RequiresApi;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.util.Date;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.*;

public class MonitoringService extends Service {

    private boolean ENABLE_MONITOR_CLIENT = false;
    private boolean ENABLE_CLEAN_SESSIONS = true;
    private boolean ENABLE_CLEAN_LOGS = true;
    private boolean ENABLE_CLEAN_HEADPOSE_DATA = true;


    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if ("kill_self".equals(intent.getAction())) {
                LogService.d(TAG, "kill self");
                killMyselfPid();
            }
        }
    };

    private Timer timer = new Timer();
    private TimerTask task = new TimerTask() {
        @RequiresApi(api = Build.VERSION_CODES.N)
        @Override
        public void run() {
            if(ENABLE_MONITOR_CLIENT) checkIsAlive();
            if(ENABLE_CLEAN_SESSIONS) cleanSessionFolder();
            if(ENABLE_CLEAN_LOGS) cleanLogFolder();
            if(ENABLE_CLEAN_HEADPOSE_DATA) cleanHeadPostDataFolder();
        }
    };

    @RequiresApi(api = Build.VERSION_CODES.N)
    public String convertTime(long time) {
        DateFormat sdf = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
        return sdf.format(new Date(time));
    }

    @RequiresApi(api = Build.VERSION_CODES.N)
    private void cleanSessionFolder() {
        File target = new File(SESSIONS_FOLDER_PATH);
        File[] result = CheckUtil.sortFolderName(target);

        if(result!= null && result.length > 10) {
            for(File folder : result) {
                LogService.d(TAG," session folder name : " + folder.getName() + " time : " + convertTime(folder.lastModified()));
            }

            for(int index = 10 ; index < result.length ; ++index) {
                LogService.i(TAG," delete session folder : " + result[index].getAbsolutePath());
                File deletetarget = new File(result[index].getAbsolutePath());
                try {
                    if(deletetarget.isDirectory())
                        FileUtils.deleteDirectory(deletetarget);
                } catch (IOException e) {
                    e.printStackTrace();
                    LogService.e(TAG,"" + e.getMessage());
                    LogService.logStackTrace(TAG, e.getStackTrace());
                }
            }
        }
    }

    private void cleanLogFolder() {
        File target = new File(LOGS_FOLDER_PATH);
        File[] result = CheckUtil.sortFolderName(target);

        if(result!= null && result.length > 10) {
            for(File folder : result) {
                LogService.d(TAG," session folder name : " + folder.getName() + " time : " + convertTime(folder.lastModified()));
            }

            for(int index = 10 ; index < result.length ; ++index) {
                File deletetarget = new File(result[index].getAbsolutePath());
                try {
                    if(deletetarget.isDirectory())
                        FileUtils.deleteDirectory(deletetarget);
                } catch (IOException e) {
                    e.printStackTrace();
                    LogService.e(TAG,"" + e.getMessage());
                    LogService.logStackTrace(TAG, e.getStackTrace());
                }
            }
        }
    }

    private void cleanHeadPostDataFolder() {
        File target = new File(ARC1_FACE_LANDMARk_FOLDER_PATH);
        File[] result = CheckUtil.sortFolderName(target);

        if(result!= null && result.length > 10) {
            for(File folder : result) {
                LogService.d(TAG," HeadPost folder name : " + folder.getName() + " time : " + convertTime(folder.lastModified()));
            }

            for(int index = 10 ; index < result.length ; ++index) {
                File deletetarget = new File(result[index].getAbsolutePath());
                try {
                    if(deletetarget.isDirectory())
                        FileUtils.deleteDirectory(deletetarget);
                } catch (IOException e) {
                    e.printStackTrace();
                    LogService.e(TAG,"" + e.getMessage());
                    LogService.logStackTrace(TAG, e.getStackTrace());
                }
            }
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.N)
    private void checkIsAlive() {
        String format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",
                Locale.getDefault()).format(new Date());
        LogService.d(TAG, "Run : " + format);

        boolean MainIsRunning = CheckUtil.isPkgRunning(
                MonitoringService.this, "com.bellus3d.android.arc.b3d4app");

        LogService.d(TAG, "MainIsRunning : " + MainIsRunning);

        if (!MainIsRunning) {
            Intent intent = new Intent();
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_CLEAR_TASK);
            intent.setClass(MonitoringService.this, MainActivity.class);
            startActivity(intent);
        }
    }


    @Override
    public void onCreate() {
        super.onCreate();
        LogService.i(TAG, "start monitor service ");
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("kill_self");
        registerReceiver(broadcastReceiver, intentFilter);
        timer.schedule(task, 0, 10000);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    private void killMyselfPid() {
        int pid = android.os.Process.myPid();
        String command = "kill -9 " + pid;
        LogService.d(TAG, "killMyselfPid : " + command);
        stopService(new Intent(MonitoringService.this, MonitoringService.class));
        try {
            Runtime.getRuntime().exec(command);
            System.exit(0);
        } catch (Exception e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(broadcastReceiver);
        if (task != null) {
            task.cancel();
        }
        if (timer != null) {
            timer.cancel();
        }
    }
}
