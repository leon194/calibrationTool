package com.bellus3d.android.b3d4.ratesttool;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class RAUtils {

    private static String TAG = "RAUtils";
    private static File RED_PATH = new File("/sys/class/leds/red/brightness");
    private static File GREEN_PATH = new File("/sys/class/leds/green/brightness");
    private static File BLUE_PATH = new File("/sys/class/leds/blue/brightness");

    public static enum LED_TYPE {
        LED_RED,
        LED_GREEN,
        LED_BLUE
    }

    public static enum LED_CONTROL {
        LED_ON,
        LED_OFF
    }

    public static boolean setLED(LED_TYPE type, LED_CONTROL ctrl ) {

        String level = ctrl.equals(LED_CONTROL.LED_ON) ? "255" : "0";
        File LED_PATH = null;
        switch (type) {
            case LED_RED:
                LED_PATH = RED_PATH;
                break;
            case LED_BLUE:
                LED_PATH = BLUE_PATH;
                break;
            case LED_GREEN:
                LED_PATH = GREEN_PATH;
                break;
            default:
                LogService.e(TAG, "Projector sys node not exist");
                break;
        }

        if(LED_PATH.exists()) {
            try {
                FileWriter fileWriter = new FileWriter(LED_PATH);
                fileWriter.write(level);
                fileWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            LogService.e(TAG, "LED sys node not exist");
            return false;
        }

        return true;
    }
}
