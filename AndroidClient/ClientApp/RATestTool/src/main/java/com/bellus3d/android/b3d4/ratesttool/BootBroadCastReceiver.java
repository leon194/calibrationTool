package com.bellus3d.android.b3d4.ratesttool;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;

public class BootBroadCastReceiver extends BroadcastReceiver {
    static final String ACTION = "android.intent.action.BOOT_COMPLETED";
    private static String TAG = "BootBroadCastReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        LogService.setLogLevel(LogService.Level.INFO);
        LogService.d(TAG,"Receive :" + intent.getAction());
        if (intent.getAction().equals(ACTION)) {
            LogService.d(TAG,"Receive boot completed");
            Intent RATestToolIntent = new Intent(context, RATestTool.class);
            RATestToolIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(RATestToolIntent);
        }
    }
}