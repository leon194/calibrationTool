package com.bellus3d.android.arc.b3d4client;

import android.util.Pair;

import java.util.Vector;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class MMFrameBuffer {
    Vector<Pair<String, Pair<Long,Long>>> mmBufferV; //<mmapfilename, <frameid,timestamp >>
    private Lock _processingLock = new ReentrantLock();
    private boolean _canRelease;
    private boolean _isProcessing;
    private String _folderName;

    public MMFrameBuffer(int size) {
        mmBufferV = new Vector<Pair<String, Pair<Long,Long>>>(size);
        _canRelease = false;
        _isProcessing = false;
        _folderName = "";
    }

    public boolean isEmpty() { return mmBufferV.isEmpty();}
    public boolean getCanRelease() { return _canRelease;}
    public void setcanRelease(boolean canRelease) { _canRelease = canRelease;}
    public boolean getisProcessing() {
        _processingLock.lock();
        try {
            return _isProcessing;
        } finally {
            _processingLock.unlock();
        }
    }
    public void setisProcessing(boolean isProcessing) {
        _processingLock.lock();
        _isProcessing = isProcessing;
        _processingLock.unlock();
    }
    public String getFolderName() { return _folderName;}
    public void setFolderName(String folderName) { _folderName = folderName;}
    public Vector<Pair<String, Pair<Long,Long>>> getMmBufferV() { return mmBufferV; }
}
