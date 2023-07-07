package com.bellus3d.android.arc.b3d4client;

import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class FPSChecker {

    private long _baseTimeStamp = 0L;
    private double _targetFPS = 0.0f;
    private int _frameCount = 0;

    public FPSChecker(double targetFPS, long baseTimeStamp) {
        this._targetFPS = targetFPS;
        this._baseTimeStamp = baseTimeStamp;
        _frameCount = 0;
    }

    public boolean init(double targetFPS, long baseTimeStamp) {
        this._targetFPS = targetFPS;
        this._baseTimeStamp = baseTimeStamp;
        _frameCount = 0;
        return (this._targetFPS == targetFPS) && (this._baseTimeStamp == baseTimeStamp) && (_frameCount == 0);
    }

    boolean isValid(long currentTimeStamp) {
        if(_targetFPS <= 0.0 ) return true;

        double duration = (currentTimeStamp - _baseTimeStamp) / 1000.0;
        if (duration != 0.0) {
            double curfps = _frameCount / duration;
            if (curfps > _targetFPS) {
                LogService.v(TAG,"curent Fps : " + curfps + ", _targetFPS : " + _targetFPS +", skip");
                return false;
            } else {
                _frameCount++;
                LogService.v(TAG, "curent Fps : " + curfps + ", _targetFPS : " + _targetFPS + ", add");
            }
        }
        return true;
    }
}
