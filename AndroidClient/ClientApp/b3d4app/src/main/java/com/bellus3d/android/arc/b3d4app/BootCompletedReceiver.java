package com.bellus3d.android.arc.b3d4app;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class BootCompletedReceiver extends BroadcastReceiver {
    static final String ACTION = "android.intent.action.BOOT_COMPLETED";

    private boolean ENABLE_AUTO_LAUNCH = false;
    @Override
    public void onReceive(Context context, Intent intent) {
        if(ENABLE_AUTO_LAUNCH) {
            LogService.v(TAG, "Receive : " + intent.getAction());
            if (intent.getAction().equals(ACTION)) {
                LogService.d(TAG, "Receive boot completed");
                Intent mainActivityIntent = new Intent(context, MainActivity.class);
                mainActivityIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(mainActivityIntent);
            }
        }
    }
}