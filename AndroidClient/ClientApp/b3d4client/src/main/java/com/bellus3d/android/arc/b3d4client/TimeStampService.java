package com.bellus3d.android.arc.b3d4client;

import android.util.Log;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class TimeStampService {

    // Timestamp methods
    private static DateFormat tsFormat = new SimpleDateFormat("yyyyMMddHHmmssSSS");

    public static String getTimestamp(Date d, long timeDiffInMilliseconds) {
        Date now = (d != null) ? d : new Date();
        return tsFormat.format(addMilliseconds(now, -timeDiffInMilliseconds));
    }

    public static Date getDateFromTimestamp(String ts) {
        if (ts == null) return new Date();

        try {
            return tsFormat.parse(ts);
        } catch (Exception e) {
            Log.d(TAG, e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return null;
        }
    }

    public static long getMillisecondsFrom(String ts_start, String ts_end) {
        Date start = getDateFromTimestamp(ts_start);
        Date end = getDateFromTimestamp(ts_end);
        return end.getTime() - start.getTime();
    }

    /**
     * @return The current formatted time
     */
    public static String getCurrentTimeStamp() {
        return DateFormat.getDateTimeInstance().format(System.currentTimeMillis());
    }

    public static Date addMilliseconds(Date d1, long ms) {
        return new Date(d1.getTime() + ms);
    }

    public Date addTime(Date d1, Date d2) {
        return addMilliseconds(d1, d2.getTime());
    }

    public Date subtractTime(Date d1, Date d2) {
        return addMilliseconds(d1, -d2.getTime());
    }
}

