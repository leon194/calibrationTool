package com.bellus3d.android.arc.b3d4client.LogService;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Created by Jia Liu on 10/4/2017.
 * A Class to log android exception crash stack trace to file.
 * Usage: Thread.setDefaultUncaughtExceptionHandler(new AndroidExceptionHandler());
 */

public class AndroidExceptionHandler implements Thread.UncaughtExceptionHandler {

    private Thread.UncaughtExceptionHandler defaultUEH;

    public AndroidExceptionHandler() {
        this.defaultUEH = Thread.getDefaultUncaughtExceptionHandler();
    }

    public void uncaughtException(Thread t, Throwable e) {
        final Writer result = new StringWriter();
        final PrintWriter printWriter = new PrintWriter(result);
        e.printStackTrace(printWriter);
        String stacktrace = result.toString();
        printWriter.close();

        LogService.logCrash(stacktrace);

        defaultUEH.uncaughtException(t, e);
    }
}