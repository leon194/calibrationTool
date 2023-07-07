package com.bellus3d.android.arc.b3d4client;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;


public class StatusLightService {

    private File _fileR;
    private File _fileG;
    private File _fileB;

    private File _fileBR_falling;
    private File _fileBR_rising;
    private File _fileBR_high;
    private File _fileBR_low;
    private File _fileBR_onoff;
    private File _fileBG_falling;
    private File _fileBG_rising;
    private File _fileBG_high;
    private File _fileBG_low;
    private File _fileBG_onoff;
    private File _fileBB_falling;
    private File _fileBB_rising;
    private File _fileBB_high;
    private File _fileBB_low;
    private File _fileBB_onoff;

    // Color state
    private int _colorR;
    private int _colorG;
    private int _colorB;

    // Color scaling to compensate for non-linear RGB luminosity
    private double _scaleR = 1.0;
    private double _scaleG = 0.2;
    private double _scaleB = 0.2;
    private double _scaleNoR = 1.0 / Math.max(_scaleG, _scaleB);

    private Light _light;

    // #FFFFFF: white,   #000000: black, #FF0000: red,
    // #00FF00: green,   #0000FF: blue,  #FFFF00: yellow
    // #FF00FF: fuchsia, #00FFFF: aqua,  #00FF40: teal
    // #00FF88: cyan
    public enum Light{
        RED(0xFF0000),
        YELLOW(0xFFFF00),
        GREEN(0x00FF00),
        WHITE(0xFFFFFF),
        BLUE(0x0000FF);

        private int color;

        Light(int color) {
            this.color = color;
        }

        public int getColor() {
            return color;
        }
    }

    public StatusLightService() {
        _fileR = new File("/sys/class/leds/red/brightness");
        _fileG = new File("/sys/class/leds/green/brightness");
        _fileB = new File("/sys/class/leds/blue/brightness");

        _fileBR_falling = new File("/sys/class/leds/red_bl/falling_time");
        _fileBR_rising = new File("/sys/class/leds/red_bl/rising_time");
        _fileBR_high = new File("/sys/class/leds/red_bl/high_time");
        _fileBR_low = new File("/sys/class/leds/red_bl/low_time");
        _fileBR_onoff = new File("/sys/class/leds/red_bl/on_off");

        _fileBG_falling = new File("/sys/class/leds/green_bl/falling_time");
        _fileBG_rising = new File("/sys/class/leds/green_bl/rising_time");
        _fileBG_high = new File("/sys/class/leds/green_bl/high_time");
        _fileBG_low = new File("/sys/class/leds/green_bl/low_time");
        _fileBG_onoff = new File("/sys/class/leds/green_bl/on_off");

        _fileBB_falling = new File("/sys/class/leds/blue_bl/falling_time");
        _fileBB_rising = new File("/sys/class/leds/blue_bl/rising_time");
        _fileBB_high = new File("/sys/class/leds/blue_bl/high_time");
        _fileBB_low = new File("/sys/class/leds/blue_bl/low_time");
        _fileBB_onoff = new File("/sys/class/leds/blue_bl/on_off");

        /* we can define use case and map to different curve */
        initBlinkingCurve(2,2,6,6);
    }
    public void setLight(Light light) {
        _light = light;
        setColor(light.getColor());
    }

    public void setBlinkLight(Light light) {
        _light = light;
        setBlink(light.getColor());
    }

    public Light getLight() {
        return _light;
    }

    private void setColor(int value) {
        _colorR = 0xff & value >> 16;
        _colorG = 0xff & value >> 8;
        _colorB = 0xff & value;
        LogService.d(TAG, "Attempting to set LED color: "+_colorR+", "+_colorG+", "+_colorB);

        resetAllBlinkLED();

        setRed(_colorR);
        setGreen(_colorG);
        setBlue(_colorB);

    }

    private void setBlink(int value) {
        _colorR = 0xff & value >> 16;
        _colorG = 0xff & value >> 8;
        _colorB = 0xff & value;
        LogService.d(TAG, "Attempting to set LED Blink color: "+_colorR+", "+_colorG+", "+_colorB);
        resetAllLED();

        if(_colorR > 0 ) setValue(_fileBR_onoff,1);
        if(_colorG > 0 ) setValue(_fileBG_onoff,1);
        if(_colorB > 0 ) setValue(_fileBB_onoff,1);
    }

    private int getColor() {
        return _colorR << 16 | _colorG << 8 | _colorB;
    }

    private void setRed(int value) {
        setValue(_fileR, scaleValue(value, _scaleR));
    }

    // Note: non-linear/geometric scaling
    private void setGreen(int value) {
        if (_colorR == 0) {
            if (_colorB == 0) {
                setValue(_fileG, value);
                return;
            }
            setValue(_fileG, scaleValue(value, _scaleG * _scaleNoR));
            return;
        }
        setValue(_fileG, scaleValue(value, _scaleG));
    }

    private void setBlue(int value) {
        if (_colorR == 0) {
            if (_colorG == 0) {
                setValue(_fileB, value);
                return;
            }
            setValue(_fileB, scaleValue(value, _scaleB * _scaleNoR));
            return;
        }
        setValue(_fileB, scaleValue(value, _scaleB));
    }

    private void setValue(File file, int value) {
        if (file.exists()) {
            try {
                if (value < 0) value = 0;
                if (value > 255) value = 255;
                String valueString = String.valueOf(value);
                FileWriter fileWriter = new FileWriter(file);
                fileWriter.write(valueString);
                fileWriter.close();
            } catch (IOException e) {
                LogService.e(TAG,"LED sys node error: "+e.toString());
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        } else {
            LogService.e(TAG,"LED sys node not exist: "+file.toString());
        }
    }

    private int scaleValue(int value, double scale) {
        int v = (int)(scale * value);
        return (v>255) ? 255 : v;
    }

    private void resetAllLED() {
        setValue(_fileB,0);
        setValue(_fileG,0);
        setValue(_fileR,0);
    }

    private void resetAllBlinkLED() {
        setValue(_fileBR_onoff,0);
        setValue(_fileBG_onoff,0);
        setValue(_fileBB_onoff,0);
    }

    private void initBlinkingCurve(int falling, int rising, int high, int low) {
        setValue(_fileBB_falling,falling);
        setValue(_fileBR_falling,falling);
        setValue(_fileBG_falling,falling);

        setValue(_fileBB_rising,rising);
        setValue(_fileBR_rising,rising);
        setValue(_fileBG_rising,rising);

        setValue(_fileBB_high,high);
        setValue(_fileBR_high,high);
        setValue(_fileBG_high,high);

        setValue(_fileBB_low,low);
        setValue(_fileBR_low,low);
        setValue(_fileBG_low,low);
    }
}
