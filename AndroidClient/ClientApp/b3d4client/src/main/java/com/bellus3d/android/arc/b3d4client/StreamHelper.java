package com.bellus3d.android.arc.b3d4client;

public class StreamHelper {
    public long streamIndex;
    public double requestedFPS;
    public long framesStreamed;
    public long tsStreamStart;
    public long framesCaptured;
    public long streamedBytes;
    public int streamHeight;
    public int streamWidth;

    public long bufferDepthRawIndex;
    public long StreamDepthDepthIndex;

    public long bufferSnapShotIndex;

    public StreamHelper() {
        streamIndex = 0;
        requestedFPS = 0.0;
        framesStreamed = 0;
        tsStreamStart = 0;
        framesCaptured = 0;
        streamedBytes = 0;
        streamHeight = 0;
        streamWidth = 0;

        bufferDepthRawIndex = 0;
        StreamDepthDepthIndex = 0;
    }
}
