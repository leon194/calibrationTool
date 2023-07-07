package com.bellus3d.android.arc.b3d4client;

import android.support.v4.util.Pair;

import java.util.Vector;

public class FrameBuffer implements Cloneable {

    private final int MAX_CACHE_SIZE_INTERNAL = 25;

    private int allocatedSize;

    private Vector<Pair<byte[], Pair<Long,Long>>> imageCache;

    private String RequestID = null;

    private boolean isAddtoNative = false;

    public enum BufferError {
        BUFFER_NO_ERROR,
        BUFFER_CACHE_FULL,
        BUFFER_CACHE_EMPTY,
        BUFFER_INVALID_INDEX
    }

    public FrameBuffer clone() throws CloneNotSupportedException {
        FrameBuffer cloneObj = new FrameBuffer(this.allocatedSize);
        cloneObj.allocatedSize = this.allocatedSize;
        cloneObj.RequestID = String.copyValueOf(this.RequestID.toCharArray());
        cloneObj.isAddtoNative = this.isAddtoNative ? true : false;
        cloneObj.imageCache = (Vector<Pair<byte[], Pair<Long,Long>>>)this.imageCache.clone();
        return cloneObj;
    }

    public FrameBuffer(int CACHE_SIZE) {
        allocatedSize = CACHE_SIZE > 25 ? MAX_CACHE_SIZE_INTERNAL : CACHE_SIZE;
        imageCache = new Vector<Pair<byte[], Pair<Long,Long>>>(allocatedSize);
    }

    public BufferError addFrame(Pair<byte[], Pair<Long,Long>> buffer) {
        if(imageCache.size() <= allocatedSize)
            imageCache.add(buffer);
        else return BufferError.BUFFER_CACHE_FULL;

        return BufferError.BUFFER_NO_ERROR;
    }

    /* get a special element from cache */
    public Pair<byte[], Pair<Long, Long>> getCache(int index) {
        if(index > allocatedSize) return null;
        else return imageCache.elementAt(index);
    }

    /* get the first element and remove it */
    Pair<byte[], Pair<Long, Long>> pop() {
        if(imageCache.isEmpty()) return null;
        else {
            Pair<byte[], Pair<Long, Long>> temp = imageCache.firstElement();
            imageCache.remove(0);
            return temp;
        }
    }

    public int get_MAX_CACHE_SIZE() {
        return MAX_CACHE_SIZE_INTERNAL;
    }

    public int getAllocatedSize() {
        return allocatedSize;
    }

    public String getRequestID() { return RequestID;};

    public void setRequestID(String requestid) { this.RequestID = requestid;};

    public boolean getBufferStatus() { return isAddtoNative;};

    public void setBufferStatus(boolean isAddtoNative) { this.isAddtoNative = isAddtoNative;};

    public boolean full() {
        return imageCache.size() == allocatedSize;
    }

    public int size() {
        return imageCache.size();
    }

    public void clear() {
        imageCache.clear();
        RequestID = null;
        isAddtoNative = false;
    }
}
