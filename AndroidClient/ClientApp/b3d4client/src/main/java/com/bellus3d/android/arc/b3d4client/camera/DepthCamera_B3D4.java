package com.bellus3d.android.arc.b3d4client.camera;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.support.v4.util.Pair;
import android.util.Range;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

import com.bellus3d.android.arc.b3d4client.ConfigService.CameraConfigService;
import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.FormattingService;
import com.bellus3d.android.arc.b3d4client.FrameBuffer;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.Queue;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_ERROR_CATEGORY.B3D_OK;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_OTHER_ERROR.ANDROID_API_ERROR;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_OTHER_ERROR.INVALID_TEXTURE_INPUT;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_OTHER_ERROR.OTHER_ERROR;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_SENSOR_STREAMING_ERROR.SENSOR_CALL_AT_INVALID_STATE;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.*;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.TripleCamResolution.RESOLUTION_2M;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.TripleCamResolution.RESOLUTION_8M;
import static com.bellus3d.android.arc.b3d4client.camera.DepthCameraError.ErrorCode.PERMISSION_ERROR;
import static com.bellus3d.android.arc.b3d4client.camera.DepthCameraState.StateType.*;

/**
 * Built-in B3D4-SPRD DepthCamera.
 */
@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
class DepthCamera_B3D4 extends DepthCameraImpl {

    Context _context;

    private static final String TAG = "DepthCamera_B3D4";

    private int CALLBACK_WIDTH =  3264;
    private int CALLBACK_HEIGHT = 2448;
    private int COLOR_WIDTH =  2448;
    private int COLOR_HEIGHT = 3264;
    private final int IR_WIDTH       = 800;
    private final int IR_HEIGHT      = 1280;
    private int COLOR_SIZE = COLOR_HEIGHT * COLOR_WIDTH * 3 / 2;
    private int TIMESTAMP_SIZE = 8;
    private int IR_SIZE = IR_HEIGHT * IR_WIDTH;
    private ImageReader mPreviewReader;  // Returns preview frame
    private ImageReader mCaptureReader;  // Returns 3 camera frames

    private String mCameraId = "17";

    File Flood = new File("/sys/flood/brightness");
    File Projector = new File("/sys/projector/brightness");

    public static DepthCameraStreamListener mDepthCameraStreamListener;

    // AE related parames
    private static float TARGET_CAPTURE_FRAME_TIME = 0f;
    private static Range<Integer> mFPSRange = new Range<>(30,30);
    private static long SENSOR_EXPOSURE_TIME = 1000000000/mFPSRange.getUpper();
    private boolean mIsCapture = false;

    private double tsArc1StreamStart = 0.0;
    private int Arc1framesStreamed = 0;


    private TextureView mPreviewTextureView;

    private Surface mPreviewSurface;

    HandlerThread mNativeHandlerThread;
    Handler mNativeHandler;
    boolean mNativeThreadControl;
    Queue<Pair<byte[], Pair<Long,Long>>> mNativeQueue;

    HandlerThread mFrameBufferHandlerThread;
    Handler mFrameBufferHandler;
    boolean mFrameBufferThreadControl;
    ConcurrentLinkedQueue<Pair<byte[], Pair<Long,Long>>> mFrameBufferQueue;


    private B3DCaptureSettings _mCaptureSettings;

    static byte[] bytereader;
    static byte[] byteDataRGB;
    static byte[] byteDataL;
    static byte[] byteDataR;

    // to sync frame data at buffer level
    private final int rawBufferCacheSize = 4;
    private boolean isRawBufferCacheInit = false;
    ArrayList<byte[]> rawBufferCache;
    private long[] timeStampMsCache;
    private long[] timeStampLsCache;
    private long[] timeStampRsCache;

    // we skip not syn frames, so need to change the skip frame number
    private final int skipFrameNum = 5;

    /* TODO clear bellow params */
    private long mCaptureStartTime;


    /**
     * A {@link Semaphore} to prevent the app from exiting before closing the camera.
     */
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);

    private Semaphore mCaptureLock = new Semaphore(1);


    /**
     * A reference to the opened {@link CameraDevice}.
     * There is only one CameraDevice
     */
    private CameraDevice mCameraDevice;

    /**
     * A {@link CameraCaptureSession } for camera preview.
     * There is only one CameraCaptureSession, used for both Preview and Capture
     */
    private CameraCaptureSession mCaptureSession;


    /**
     * {@link CaptureRequest.Builder} for the camera preview
     * created and set in "createCameraPreview"
     */
    private CaptureRequest.Builder mPreviewRequestBuilder;


    DepthCamera_B3D4(final Context context, B3DDepthCamera.DeviceType _DepthCamType) {
        super(_DepthCamType);
        LogService.d(TAG, "Creating B3D4 DepthCamera");
        DepthCameraJNI(B3DDepthCamera.DeviceType.DEVICE_B3D4.ordinal());
        _context = context;

        mCurrentStateType = CONNECTED;

        mPreviewStatus = B3DDepthCamera.PreviewStatus.STOPPED;

        mActivityContext = context;

        mStreamStartTime = 0;
        mTotalFramesStreamed = 0;
        mStreamedTotalTime = 0 ;

        /* TODO clear bellow params */
        mTotalFramesCaptured = 0;
    }

    @Override
    public B3D4ClientError setPreviewTexture(TextureView previewTexture, TextureView testTexture) {
        LogService.v(TAG, " called");

        if (previewTexture == null) {
            String errorMessage = LogService.getClassNameAndMethodName() + " error - Input TextureView is null";
            return new B3D4ClientError(INVALID_TEXTURE_INPUT.getValue());
        }

        if (!previewTexture.isAvailable()) {
            LogService.w(TAG, "input TextureView not available yet");
        }

        mPreviewTextureView = previewTexture;

        return new B3D4ClientError(B3D_OK.getValue());
    }


    public B3D4ClientError connectSync() {
        LogService.d(TAG, " called");
        updateCameraState(CONNECTED);
        return new B3D4ClientError(B3D_OK.getValue());
    }

    public B3D4ClientError openSync() {

        String methodName = LogService.getClassNameAndMethodName();

        startBackgroundThreads();

        if (mCurrentStateType == OPEN) {
            LogService.w(TAG, "DepthCamera is already OPEN. no-op.");
            return new B3D4ClientError(B3D_OK.getValue());
        }

        // Check acceptable state
        if (mCurrentStateType != CONNECTED) {
            String errorMessage = methodName + " called at invalid state " + mCurrentStateType;
            LogService.e(TAG, errorMessage);
            return new B3D4ClientError(SENSOR_CALL_AT_INVALID_STATE.getValue(),errorMessage);
        }

        updateCameraState(OPENING);

        try {
            // Initialize Camera Manager
            CameraManager cameraManager = (CameraManager) mActivityContext.getSystemService(Context.CAMERA_SERVICE);
            if (cameraManager == null) {
                String errorMessage = methodName + " error - cannot get CameraManager";
                LogService.e(TAG, errorMessage);
                return new B3D4ClientError(ANDROID_API_ERROR.getValue(),errorMessage);
            }

            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                String errorMessage = methodName + "  error - time out waiting to tryAcquire camera";
                LogService.e(TAG, errorMessage);
                return new B3D4ClientError(OTHER_ERROR.getValue(),errorMessage);
            }
            cameraManager.openCamera(mCameraId, mStateCallback, mCaptureHandler);
        }
        catch (CameraAccessException | SecurityException e) {
            String errMsg = methodName + " error - no permission to access camera";
            LogService.e(TAG, errMsg);
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return new B3D4ClientError(ANDROID_API_ERROR.getValue(),errMsg);
        }
        catch (InterruptedException e) {
            String errMsg = methodName + " error - interrupted during tryAcquire camera.";
            LogService.e(TAG, errMsg);
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return new B3D4ClientError(ANDROID_API_ERROR.getValue(),errMsg);
        }

        final int ATTEMPT_INTERVAL = 30;  // check every N ms to see if camera is opened
        final int MAX_ATTEMPT      =  5;  // fail camera open in 3 seconds
        int openAttempts = 0;  // keep checking if Android camera is opened

        while ((mCameraDevice == null)) {
            try {
                openAttempts++;
                Thread.sleep(ATTEMPT_INTERVAL);
                LogService.d(TAG, "check open attempt: " + openAttempts);
            }
            catch (InterruptedException e) {
                String errMsg = methodName + " error - Current thread is interrupted.";
                LogService.e(TAG, errMsg);
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
                return new B3D4ClientError(OTHER_ERROR.getValue(),errMsg);
            }
        }
        LogService.d(TAG, methodName + "Finished waiting openCamera() callback");

        // Check if openCamera() succeeds or not
        if (mCameraDevice != null) {
            // openSync finished successfully
            updateCameraState(OPEN);
            LogService.d(TAG, methodName + " successful");
            return new B3D4ClientError(B3D_OK.getValue());
        }
        else {
            updateCameraState(CONNECTED);
            String errorMessage = methodName + " error - CameraManager.openCamera() failed";
            LogService.e(TAG, errorMessage);
            return new B3D4ClientError(ANDROID_API_ERROR.getValue(),errorMessage);
        }
    }


    public B3D4ClientError closeSync() {

        String methodName = LogService.getClassNameAndMethodName();

        // Check acceptable state
        if (mCurrentStateType == CONNECTED) {
            LogService.w(TAG, " DepthCamera is already close. no-op.");
            return new B3D4ClientError(B3D_OK.getValue());
        }

        if (mCurrentStateType != OPEN) {
            String errorMessage = methodName + " called at invalid state " + mCurrentStateType;
            LogService.e(TAG, errorMessage);
            return new B3D4ClientError(SENSOR_CALL_AT_INVALID_STATE.getValue(),errorMessage);
        }

        updateCameraState(CLOSING);

        try {
            mCameraOpenCloseLock.acquire();

            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }

        } catch (InterruptedException e) {
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            String errorMsg = methodName + " error - camera lock acquire was interrupted.";
            return new B3D4ClientError(OTHER_ERROR.getValue(),
                    errorMsg);

        } finally {
            mCameraOpenCloseLock.release();
            stopBackgroundThreads();
            updateCameraState(CONNECTED);
        }

        CameraConfigService.set("persist.camera.ov8856.shutter", String.valueOf(0));
        CameraConfigService.set("persist.camera.ov8856.gain", String.valueOf(0));
        CameraConfigService.set("persist.vendor.camera.bypass.ae", String.valueOf(0));
        CameraConfigService.set("persist.vendor.camera.bypass.awb", String.valueOf(0));
        CameraConfigService.set("debug.isp.awb.rgain.set", String.valueOf(0));
        CameraConfigService.set("debug.isp.awb.ggain.set", String.valueOf(0));
        CameraConfigService.set("debug.isp.awb.bgain.set", String.valueOf(0));

        return new B3D4ClientError(B3D_OK.getValue());
    }

    public B3D4ClientError startStreamSync() {

        String methodName = LogService.getClassNameAndMethodName();
        tsArc1StreamStart = System.currentTimeMillis();
        Arc1framesStreamed = 0;
        isRawBufferCacheInit = false;
        rawBufferCache = new ArrayList<byte[]>(rawBufferCacheSize);
        timeStampMsCache = new long[rawBufferCacheSize];
        timeStampLsCache = new long[rawBufferCacheSize];
        timeStampRsCache = new long[rawBufferCacheSize];

        // Check acceptable states
        if (mCurrentStateType == STREAMING ||
                mCurrentStateType == STARTING) {

            LogService.w(TAG, " DepthCamera is already STARTING or STREAMING. no-op.");
            return new B3D4ClientError(B3D_OK.getValue());
        }

        if (mCurrentStateType != OPEN) {
            String errorMessage = methodName + " called at invalid state " + mCurrentStateType;
            LogService.e(TAG, errorMessage);
            return new B3D4ClientError(SENSOR_CALL_AT_INVALID_STATE.getValue(),errorMessage);
        }


        updateCameraState(STARTING);

        // reset FPS controls
        mTotalFramesStreamed = 0;
        mStreamedTotalTime = 0;
        mTotalFramesCaptured = 0;

        mCaptureHandler.post(new Runnable() {
            @Override
            public void run() {
                // only create CameraCaptureSession if it hasn't been done
                if (mCaptureSession == null) {
                    allocateCameraBuffers(B3DDepthCamera.CaptureSessionMode.STREAM);
                    // createCameraCaptureSession() will be triggered after allocation
                }
                else {
                    // CameraCaptureSession has already been created
                    runStreamProgress();
                }
            }
        });




        // will trigger mCaptureReader -> framesAvailableListener
        // Probably should just do a while loop here to wait for camera open?
        // There should also has a timeout setting
        // final Handler openCameraHandler = new Handler(Looper.myLooper());
        final int ATTEMPT_INTERVAL = 50;  // check every N ms to see if camera is opened
        final int MAX_ATTEMPT      = 60;  // fail camera open in 3 seconds
        int streamAttempts        =  0;  // keep checking if Android camera is opened

        // If mTotalFramesStreamed == 0, and stream attempts < MAX
        // Keep checking
        while ((mTotalFramesStreamed == 0) && (streamAttempts < MAX_ATTEMPT)) {

            try {
                streamAttempts++;
                Thread.sleep(ATTEMPT_INTERVAL);
                LogService.d(TAG, "check stream attempt: " + streamAttempts);
            }
            catch (InterruptedException e) {
                String errMsg = methodName + " error - Current thread is interrupted.";
                LogService.e(TAG, errMsg);
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
                return new B3D4ClientError(OTHER_ERROR.getValue(),errMsg);
            }
        }
        LogService.d(TAG, methodName + "Finished waiting startStream() callback");

        // Check if start streaming succeeds or not
        if (mTotalFramesStreamed > 0) {
            mCaptureStartTime = System.currentTimeMillis();
            // openSync finished successfully

            if( mActivityContext.getPackageName().equals("com.bellus3d.android.calibrationtool") ||
                mActivityContext.getPackageName().equals("com.bellus3d.android.b3d4.ratesttool")) {
                mNativeHandlerThread = new HandlerThread("NativeHandler");
                mNativeHandlerThread.start();
                mNativeHandler = new Handler(mNativeHandlerThread.getLooper());
                mNativeQueue = new LinkedList<Pair<byte[], Pair<Long, Long>>>();
                mNativeThreadControl = true;
                mNativeHandler.post(processNative);
            }

            updateCameraState(STREAMING);
            LogService.d(TAG, methodName + " successful");
            return new B3D4ClientError(B3D_OK.getValue());
        }
        else {
            updateCameraState(OPEN);
            closeSync();
            String errorMessage = methodName + " failed - attempt to stream timed out";
            LogService.e(TAG, errorMessage);
            return new B3D4ClientError(OTHER_ERROR.getValue(),errorMessage);
        }
    }


    @Override
    public B3D4ClientError stopStreamSync() {

        String methodName = LogService.getClassNameAndMethodName();
        // Check acceptable states
        if (mCurrentStateType == OPEN) {
            LogService.w(TAG, " DepthCamera is already OPEN. no-op.");
            return new B3D4ClientError(B3D_OK.getValue());
        }

        // if mNativeThreadControl is true , mNativeHandlerThread is still running
        if (mCurrentStateType == CONNECTED && !mNativeThreadControl) {
            LogService.w(TAG, " DepthCamera is connected. no-op.");
            return new B3D4ClientError(B3D_OK.getValue());
        }

        // for cancel processing, it might receive cancel and need to stop when
        // mCurrentStateType is STARTING
        if ( !(mCurrentStateType == STREAMING || mCurrentStateType == STARTING) ) {

            String errorMsg = methodName + " called at invalid state: " + mCurrentStateType;
            LogService.e(TAG, errorMsg);
            return new B3D4ClientError(SENSOR_CALL_AT_INVALID_STATE.getValue(),errorMsg);
        }

        if(mNativeHandlerThread != null)
            mNativeHandlerThread.quitSafely();
        mNativeThreadControl = false;

        // only arc1/stream_capture (buffer_cancel --> need to re-write and test further)
        // and cancel_process need to stop mFrameBufferHandlerThread here
        // arcx will/need stop by its self
        if(_mCaptureSettings != null && (
                _mCaptureSettings.getRequest().toLowerCase().equals(STRING_STREAM_CAPTURE) ||
                _mCaptureSettings.getRequest().toLowerCase().equals(STRING_CANCEL_PROCESS))
        ) {
            if (mFrameBufferHandlerThread != null) {
                mFrameBufferHandlerThread.quitSafely();
                mFrameBufferHandlerThread = null;
            }

            mFrameBufferThreadControl = false;

            rawBufferCache.clear();
            isRawBufferCacheInit = false;
            timeStampMsCache = timeStampLsCache = timeStampRsCache = null;
        }
        updateCameraState(STOPPING);

        try {

            LogService.v(TAG, "Stop repeating");

            mCaptureSession.stopRepeating();  // <- stop taking pictures continuously
            mCaptureSession.abortCaptures();
            mCameraOpenCloseLock.acquire();
            if (null != mCaptureSession) {
                mCaptureSession.close();
                mCaptureSession = null;
            }

            if(null != mPreviewTextureView)
                mPreviewTextureView = null;

            updateCameraState(OPEN);


        } catch (CameraAccessException | InterruptedException e) {
            updateCameraState(CONNECTED);
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            // throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
        }

        return new B3D4ClientError(B3D_OK.getValue());
    }

    @Override
    B3D4ClientError setCaptureEvent(B3DCaptureSettings settings, B3DDepthCamera.CaptureEvent CEvent) {
        if(!(mActivityContext.getPackageName().equals("com.bellus3d.android.calibrationtool") ||
                mActivityContext.getPackageName().equals("com.bellus3d.android.b3d4.ratesttool"))) {
            // check mFrameBufferHandlerThread again to avoid we left previous thread still running
            if (mFrameBufferHandlerThread != null) {
                mFrameBufferHandlerThread.quitSafely();
                mFrameBufferHandlerThread = null;
            }
            switch (CEvent) {
                // no need to start when is calibrationtool and ratesttool
                case CAPTURE_BUFFER_2M: {
                    CALLBACK_WIDTH =  2048;
                    CALLBACK_HEIGHT = 1536;
                    COLOR_WIDTH =  1224;
                    COLOR_HEIGHT = 1632;
                    bytereader = null;
                    bytereader = new byte[ARC_2M_TIMESTAMP_IRR_END];

                    /* 2M resolution currently use for stream_start to do face tracking, but if we need to buffer_capture with
                     * 2M resolution, still can support it
                     *  */
                    if(settings.getRequest().equals(STRING_STREAM_START) && settings.isTrackingFace()) {
                        //TODO
                    } else if(settings.getRequest().equals(STRING_STREAM_CAPTURE)) {
                        mFrameBufferHandlerThread = new HandlerThread("Arc1Handler");
                        mFrameBufferHandlerThread.start();
                        mFrameBufferHandler = new Handler(mFrameBufferHandlerThread.getLooper());
                        mFrameBufferQueue = new ConcurrentLinkedQueue<Pair<byte[], Pair<Long, Long>>>();
                        mFrameBufferThreadControl = true;
                        mFrameBufferHandler.post(processArc1);
                    } else if(settings.getRequest().equals(STRING_BUFFER_CAPTURE)) {
                        mFrameBufferHandlerThread = new HandlerThread("ArcxHandler-4DCapture");
                        mFrameBufferHandlerThread.start();
                        mFrameBufferHandler = new Handler(mFrameBufferHandlerThread.getLooper());
                        mFrameBufferQueue = new ConcurrentLinkedQueue<Pair<byte[], Pair<Long, Long>>>();
                        mFrameBufferThreadControl = true;
                        mFrameBufferHandler.post(processArcX);
                    }
                }
                break;
                case CAPTURE_BUFFER_8M: {
                    CALLBACK_WIDTH =  3264;
                    CALLBACK_HEIGHT = 2448;
                    COLOR_WIDTH =  2448;
                    COLOR_HEIGHT = 3264;
                    bytereader = null;
                    bytereader = new byte[ARC_8M_TIMESTAMP_IRR_END];

                    if(settings.getRequest().equals(STRING_BUFFER_CAPTURE)) {
                        mFrameBufferHandlerThread = new HandlerThread("ArcxHandler-StillCapture");
                        mFrameBufferHandlerThread.start();
                        mFrameBufferHandler = new Handler(mFrameBufferHandlerThread.getLooper());
                        mFrameBufferQueue = new ConcurrentLinkedQueue<Pair<byte[], Pair<Long, Long>>>();
                        mFrameBufferThreadControl = true;
                        mFrameBufferHandler.post(processArcX);
                    } else if(settings.getRequest().equals(STRING_STREAM_CAPTURE)) {
                        mFrameBufferHandlerThread = new HandlerThread("Arc1Handler");
                        mFrameBufferHandlerThread.start();
                        mFrameBufferHandler = new Handler(mFrameBufferHandlerThread.getLooper());
                        mFrameBufferQueue = new ConcurrentLinkedQueue<Pair<byte[], Pair<Long, Long>>>();
                        mFrameBufferThreadControl = true;
                        mFrameBufferHandler.post(processArc1);
                    } else if(settings.getRequest().equals(STRING_STREAM_DEPTH)) {
                        mFrameBufferHandlerThread = new HandlerThread("ArcxHandler-MotionCapture");
                        mFrameBufferHandlerThread.start();
                        mFrameBufferHandler = new Handler(mFrameBufferHandlerThread.getLooper());
                        mFrameBufferQueue = new ConcurrentLinkedQueue<Pair<byte[], Pair<Long, Long>>>();
                        mFrameBufferThreadControl = true;
                        mFrameBufferHandler.post(processArcX);
                    }
                }
                break;
            }
        } else {
            /* calibrationTool and ratestTool part */
            switch (CEvent) {
                case CAPTURE_BUFFER_2M: {
                    CALLBACK_WIDTH = 2048;
                    CALLBACK_HEIGHT = 1536;
                    COLOR_WIDTH = 1224;
                    COLOR_HEIGHT = 1632;
                    bytereader = null;
                    bytereader = new byte[6291456];
                }
                break;
                case CAPTURE_BUFFER_8M: {
                    CALLBACK_WIDTH = 3264;
                    CALLBACK_HEIGHT = 2448;
                    COLOR_WIDTH = 2448;
                    COLOR_HEIGHT = 3264;
                    bytereader = null;
                    bytereader = new byte[15980544];
                }
                break;
                case CAPTURE_SINGLE: {
                    try {
                        mCaptureLock.acquire();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        LogService.logStackTrace(TAG, e.getStackTrace());
                    }

                    mIsCapture = true;
                    mCaptureLock.release();
                }
                break;
            }
        }

        /* update size again */
        COLOR_SIZE = COLOR_HEIGHT * COLOR_WIDTH * 3 / 2;
        IR_SIZE = IR_HEIGHT * IR_WIDTH;

        return new B3D4ClientError(B3D_OK.getValue());
    }


    public void registerStreamListener(DepthCameraStreamListener listener) {
        mDepthCameraStreamListener = listener;
    }

    @Override
    void unregisterStreamListener(DepthCameraStreamListener listener) {
        mDepthCameraStreamListener = null;
    }

    @Override
    void setPreviewTarget(TextureView viewL, TextureView viewR, TextureView viewM) {
        try {

            if (viewL == null || viewR == null || viewM == null) {
                throw new Exception("TextureView L / R / M is null");
            }

            if (!viewL.isAvailable() || !viewR.isAvailable() || !viewM.isAvailable()) {
                throw new Exception("TextureView L / R / M not available");
            }

            Surface surfaceL = new Surface(viewL.getSurfaceTexture());
            Surface surfaceR = new Surface(viewR.getSurfaceTexture());
            Surface surfaceM = new Surface(viewM.getSurfaceTexture());

            DepthCameraImpl.setPreviewTargetJNI(surfaceL, surfaceR, surfaceM, DepthCamera_B3D4.this);

        }
        catch (Exception e) {
            LogService.e(TAG, "failed : " + e.getMessage());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
        LogService.d(TAG, " success ");
    }

    @Override
    void setCameraID(String cameraID) {
    }

    @Override
    Size getSize() {
        return new Size(COLOR_WIDTH,COLOR_HEIGHT);
    }

    @Override
    long getCaptureMilliseconds() {
        return mStreamedTotalTime;
    }

    @Override
    long getProcessMilliseconds() {
        return 0;
    }

    @Override
    boolean setFlood(B3DDepthCamera.Control ctrl, String value) {
        if(Flood.exists()) {

            int curValue = 0;
            String curLevel = "";
            try {
                FileReader fr = new FileReader(Flood);
                curValue = fr.read();
                while(curValue != -1) {
                    curLevel += String.valueOf((char)curValue);
                    curValue = fr.read();
                }
                fr.close();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
            //LogService.e(TAG,"leon -->" + curLevel);

            if(curLevel != value) {
                try {
                    FileWriter fileWriter = new FileWriter(Flood);
                    fileWriter.write(value);
                    fileWriter.close();
                } catch (IOException e) {
                    e.printStackTrace();
                    LogService.logStackTrace(TAG, e.getStackTrace());
                }
            } else {
                LogService.i(TAG,"skip same value");
            }
        } else {
            LogService.e(TAG, "Flood sys node not exist");
            return false;
        }
        return true;
    }

    @Override
    boolean setProject(B3DDepthCamera.Control ctrl, String value) {
        if(Projector.exists()) {
            try {
                FileWriter fileWriter = new FileWriter(Projector);
                fileWriter.write(value);
                fileWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        } else {
            LogService.e(TAG, "Projector sys node not exist");
            return false;
        }
        return true;
    }

    @Override
    DepthCameraError setFps(Range<Integer> fps) {
        mFPSRange = fps;
        SENSOR_EXPOSURE_TIME = 1000000000/fps.getUpper();
        return null;
    }

    @Override
    float getFps() {
        return mTotalFramesStreamed / mStreamedTotalTime;
    }

    @Override
    void setCaptureSettings(B3DCaptureSettings mCaptureSetting) {
        _mCaptureSettings = mCaptureSetting;
    }

    /**
     * Clear timestamps
     */
    public void resetFrameTime() {
        mCaptureStartTime = System.currentTimeMillis();
    }


    private void allocateCameraBuffers(final B3DDepthCamera.CaptureSessionMode captureMode) {
        final String methodName = LogService.getClassNameAndMethodName();
        LogService.d(TAG, " called");

        //  ========  Allocate buffer for camera STREAMING
        if(mCaptureReader != null){
            mCaptureReader.close();
        }
        mCaptureReader = ImageReader.newInstance(CALLBACK_WIDTH,CALLBACK_HEIGHT,ImageFormat.RAW_SENSOR, 1);
        mCaptureReader.setOnImageAvailableListener(framesAvailableListener, null);


        /* Allocate buffer for camera PREVIEW */
        /* camera id 17 need mPreviewReader to open camera and streaming, event we didn't use mPreviewReader */
        if(mPreviewReader != null){
            mPreviewReader.close();
        }

        mPreviewReader = ImageReader.newInstance(COLOR_WIDTH,COLOR_HEIGHT,ImageFormat.JPEG, 1);



        // When "mPreviewTextureView" is available, createCameraPreviewSession
        if (mPreviewTextureView != null && mPreviewTextureView.isAvailable()) {
            LogService.d(TAG, " previewTextureView.isAvailable");
            createCameraCaptureSession(captureMode);
        }
        else {
            LogService.d(TAG, " previewTextureView not Available");

            TextureView.SurfaceTextureListener surfaceTextureListener = new TextureView.SurfaceTextureListener() {
                @Override
                public void onSurfaceTextureAvailable(SurfaceTexture texture, int width, int height) {
                    LogService.d(TAG, "onSurfaceTextureAvailable()");
                    createCameraCaptureSession(captureMode);
                }

                @Override
                public void onSurfaceTextureSizeChanged(SurfaceTexture texture, int width, int height) {
                }

                @Override
                public boolean onSurfaceTextureDestroyed(SurfaceTexture texture) {
                    LogService.d(TAG, "onSurfaceTextureDestroyed");
                    // Clear consumer of buffer here
                    mPreviewTextureView = null;
                    return true;
                }

                @Override
                public void onSurfaceTextureUpdated(SurfaceTexture texture) {
                    LogService.d(TAG, "onSurfaceTextureUpdated");
                }
            };
            mPreviewTextureView.setSurfaceTextureListener(surfaceTextureListener);
        }
    }


    private B3D4ClientError createCameraCaptureSession(final B3DDepthCamera.CaptureSessionMode captureMode) {
        final String methodName = LogService.getClassNameAndMethodName();
        LogService.d(TAG, " called");


        SurfaceTexture previewSurfaceTexture = mPreviewTextureView.getSurfaceTexture();
        if (previewSurfaceTexture == null) {
            String errMessage = " error - previewTextureView invalid (SurfaceTexture null)";
            reportUpdateSync(OPEN);
            return new B3D4ClientError(INVALID_TEXTURE_INPUT.getValue());
        }

        // Creating Preview Surface object
        previewSurfaceTexture.setDefaultBufferSize(COLOR_WIDTH,COLOR_HEIGHT);
        if (mPreviewSurface != null) mPreviewSurface.release();
        mPreviewSurface = new Surface(previewSurfaceTexture);

        //  ======== Create Capture Session  ========  //
        try {
            mCameraDevice.createCaptureSession(

                    // 1st Argument: Allocate Preview & Capture buffers for "CaptureSession"
                    // List<Surface> - output targets for captured image data
                    Arrays.asList(
                            mPreviewSurface,
                            mPreviewReader.getSurface(),
                            mCaptureReader.getSurface()
                    ),

                    // 2nd Argument: StateCallback
                    new CameraCaptureSession.StateCallback() {
                        @Override
                        public void onConfigured(@NonNull CameraCaptureSession capSession) {
                            // When the session is ready, we start displaying the preview.
                            LogService.d(TAG, "CaptureSession created");

                            mCaptureSession = capSession;

                            switch (captureMode) {
                                case PREVIEW:
                                    runPreviewProgress();
                                    break;
                                case STREAM:
                                    runStreamProgress();
                                    break;
                            }
                        }

                        @Override
                        public void onConfigureFailed(@NonNull CameraCaptureSession capSession) {
                            reportErrorToAllObservers(
                                    new DepthCameraError(DepthCameraError.ErrorCode.INTERNAL_LIBRARY_ERROR,
                                            methodName + " onConfigureFailed()"));
                            reportUpdateSync(OPEN);
                        }
                    },  // end of constructing callback object

                    // 3rd Argument: handler - on which thread callback will be invoked
                    null

            );  // End of mCameraDevice.createCaptureSession()

        } catch (CameraAccessException e) {
            String errMsg = "error - no permission to access camera";
            reportErrorToAllObservers(new DepthCameraError(PERMISSION_ERROR, errMsg));
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return new B3D4ClientError(ANDROID_API_ERROR.getValue(),
                    errMsg);
        }

        return new B3D4ClientError(B3D_OK.getValue());
    }



    private void runPreviewProgress() {

        LogService.d(TAG," called");
        String methodName = LogService.getClassNameAndMethodName();

        try {

            mPreviewRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);

            mPreviewRequestBuilder.addTarget(mCaptureReader.getSurface());
            mPreviewRequestBuilder.addTarget(mPreviewSurface);

            // Auto focus is turned off for using back camera, always want to focus on the front area which is the face
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_OFF);
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AWB_MODE, CaptureRequest.CONTROL_AWB_MODE_AUTO);

            mStreamAvaliableTime = mStreamStartTime = System.currentTimeMillis();

            // Finally, we start displaying the camera preview.
            mCaptureSession.setRepeatingRequest(mPreviewRequestBuilder.build(), null, null);

            // Preview control logic
            if (mPreviewStatus == B3DDepthCamera.PreviewStatus.STOPPING) {
                LogService.d(TAG, methodName + " Preview stopping");
                stopStreamSync();
            }
            else {
                mPreviewStatus = B3DDepthCamera.PreviewStatus.ONGOING;
                LogService.d(TAG, methodName + " Preview ongoing");
            }

        } catch (CameraAccessException e) {
            String errMsg = methodName + "error - no permission to access camera";
            reportErrorToAllObservers(new DepthCameraError(PERMISSION_ERROR, errMsg));
            reportUpdateSync(OPEN);
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    private void runStreamProgress() {

        LogService.d(TAG," called");

        String methodName = LogService.getClassNameAndMethodName();

        try {

            mPreviewRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);

            mPreviewRequestBuilder.addTarget(mCaptureReader.getSurface());
            mPreviewRequestBuilder.addTarget(mPreviewSurface);

            mStreamAvaliableTime = mStreamStartTime = System.currentTimeMillis();

            CameraCaptureSession.CaptureCallback CaptureCallback = new CameraCaptureSession.CaptureCallback() {

                @Override
                public void onCaptureCompleted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull TotalCaptureResult result) {
                    LogService.d(TAG, "CameraCaptureSession: "+session + "CaptureRequest: " + request + "TotalCaptureResult : " + result);
                }
            };

            // Finally, we start displaying the camera preview.
            mCaptureSession.setRepeatingRequest(mPreviewRequestBuilder.build(), null, null);

        } catch (CameraAccessException e) {
            String errMsg = methodName + "error - no permission to access camera";
            reportErrorToAllObservers(new DepthCameraError(PERMISSION_ERROR, errMsg));
            reportUpdateSync(OPEN);
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }
    }

    private CaptureRequest.Builder captureRequestBuilder;

    /**
     * {@link CameraDevice.StateCallback} is called when {@link CameraDevice} changes its state.
     */
    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            // This method is called when the camera is opened.  We start camera preview here.
            mCameraOpenCloseLock.release();
            mCameraDevice = cameraDevice;

            LogService.d(TAG, "succeeded.");
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;

            LogService.e(TAG, "error - from Android Camera API: " + error);
        }

    };

    private ImageReader.OnImageAvailableListener framesAvailableListener = new ImageReader.OnImageAvailableListener() {
        @RequiresApi(api = Build.VERSION_CODES.N)
        @Override
        public void onImageAvailable(ImageReader reader){

            LogService.v(TAG, "");
            final long currentTimeStamp = System.currentTimeMillis() - mCaptureStartTime;
            mStreamedTotalTime += System.currentTimeMillis() - mStreamAvaliableTime;
            mStreamAvaliableTime = System.currentTimeMillis();

            Image image = null;
            try {

                image = reader.acquireNextImage();

                if (image == null) {
                    LogService.e(TAG, " image is null");
                    return;
                }

                double requestedFPS = 5.0;
                if (requestedFPS != 0.0 && _mCaptureSettings != null &&
                        _mCaptureSettings.getRequest().toLowerCase().equals(STRING_STREAM_CAPTURE)) {
                    double duration = (System.currentTimeMillis() - tsArc1StreamStart) / 1000.0;
                    if (duration != 0.0) {
                        double fps = Arc1framesStreamed / duration;
                        if (fps > requestedFPS) {
                            image.close();
                            LogService.v(TAG,"Arc1 fps control, fps " + fps);
                            return;
                        }
                    }
                }

                if(mActivityContext.getPackageName().equals("com.bellus3d.android.b3d4.ratesttool")) {
                    /* RA Test Tool only need return null to count the frame number */
                    mDepthCameraStreamListener.onFrameNative(null, mTotalFramesStreamed, currentTimeStamp, RESOLUTION_8M.ordinal(), false);
                    image.close();
                    LogService.i(TAG," RA Test  frame count : " + mTotalFramesStreamed);
                    return;
                }

                // Obtain pixel bytes from "ByteBuffer"
                ByteBuffer buffer = image.getPlanes()[0].getBuffer();
                buffer.get(bytereader);
                LogService.v(TAG, "byteData buffer length:" + bytereader.length);

                long timeStampM = 0L, timeStampL = 0L, timeStampR = 0L;
                try {
                    /* c timestamp is big endian, java byte array is small endian, so need to reverse it */
                    timeStampM = ByteBuffer.wrap((FormattingService.byteArrayReverse(Arrays.copyOfRange(bytereader, COLOR_SIZE + IR_SIZE *2, COLOR_SIZE + IR_SIZE *2 + 8)))).getLong();
                    timeStampL = ByteBuffer.wrap((FormattingService.byteArrayReverse(Arrays.copyOfRange(bytereader, COLOR_SIZE + IR_SIZE *2 + 8, COLOR_SIZE + IR_SIZE *2 + 16)))).getLong();
                    timeStampR = ByteBuffer.wrap((FormattingService.byteArrayReverse(Arrays.copyOfRange(bytereader, COLOR_SIZE + IR_SIZE *2 + 16, COLOR_SIZE + IR_SIZE *2 + 24)))).getLong();
                    LogService.i(TAG, "incoming use frame [M,L,R] : [" + timeStampM + "," + timeStampL + "," + timeStampR + "]");
                    LogService.i(TAG, "incoming frame diff [M-L,L-R] : [" + (timeStampM - timeStampL) / NSTOMS + "," + (timeStampL - timeStampR) / NSTOMS + "]");

                } catch (Exception e) {
                    LogService.w(TAG," might not have timestamp info");
                    e.printStackTrace();
                    LogService.logStackTrace(TAG, e.getStackTrace());
                }

                Arc1framesStreamed++;
                mTotalFramesStreamed++;

                /* if is calibrationTool or ratestTool */
                if( mNativeQueue != null &&
                        ( mActivityContext.getPackageName().equals("com.bellus3d.android.calibrationtool") ||
                          mActivityContext.getPackageName().equals("com.bellus3d.android.b3d4.ratesttool"))
				  ) {
                    mNativeQueue.add(new Pair<byte[], Pair<Long, Long>>(bytereader.clone(),new Pair<Long, Long> (mTotalFramesStreamed,currentTimeStamp)));
                }


                final long acceptedFrameId = mTotalFramesStreamed - skipFrameNum;

                // we wish to skip 5 frames, and rawBufferCache is 4, start collect frame at acceptedFrameId >= -4
                if (!isRawBufferCacheInit && acceptedFrameId >= -4 && rawBufferCache.size() < rawBufferCacheSize) {
                    rawBufferCache.add(bytereader.clone());
                    image.close();
                    if(rawBufferCache.size() == rawBufferCacheSize) isRawBufferCacheInit = true;
                    return;
                }

                if(acceptedFrameId >= 0)
                    mDepthCameraStreamListener.onNotificationHost(_mCaptureSettings, timeStampL /NSTOMS, timeStampR /NSTOMS, acceptedFrameId);

                /* add buffer to bufferCache */
                if (_mCaptureSettings != null) {
                    String action = _mCaptureSettings.getRequest().toLowerCase();
                    switch (action) {
                        case STRING_STREAM_START : {
                            if(_mCaptureSettings.getStreamFrames() < 0) {
                                // if getBufferFrames < 0 only return ok to host
                                if(mTotalFramesCaptured == 0) {
                                    mDepthCameraStreamListener.onStatusHost(_mCaptureSettings, NetWorkMessage.Status.OK.getName());
                                }
                            } else {
                                mDepthCameraStreamListener.onFrameHost(_mCaptureSettings, bytereader.clone(), mTotalFramesCaptured);
                                mTotalFramesCaptured++;
                            }
                            break;
                        }
                        case STRING_STREAM_CAPTURE: {
                            if( (_mCaptureSettings.getDepthFrames() + rawBufferCacheSize ) < mTotalFramesCaptured) {
                                LogService.v(TAG,"frame enough, current frame id " + mTotalFramesCaptured +
                                        ", required depthFrames : " + _mCaptureSettings.getDepthFrames());
                                stopStreamSync();
                                setProject(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                setFlood(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                closeSync();
                            } else {
                                if (mFrameBufferQueue != null && acceptedFrameId >= 0) {
                                    LogService.d(TAG, "add frame to queue " + mTotalFramesCaptured);
                                    mFrameBufferQueue.offer(new Pair<byte[], Pair<Long, Long>>(bytereader.clone(), new Pair<Long, Long>(mTotalFramesCaptured, currentTimeStamp)));
                                    mTotalFramesCaptured++;
                                }
                            }
                            break;
                        }
                        case STRING_STREAM_DEPTH: {
                            if( (_mCaptureSettings.getStreamDepthFrames() + rawBufferCacheSize )  < mTotalFramesCaptured ) {
                                LogService.v(TAG,"frame enough, current frame id " + mTotalFramesCaptured +
                                        ", required depthFrames : " + _mCaptureSettings.getStreamDepthFrames() + " rawBufferCacheSize : " + rawBufferCacheSize);
                                stopStreamSync();
                                setProject(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                setFlood(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                closeSync();
                                mDepthCameraStreamListener.onStatusHost(_mCaptureSettings, NetWorkMessage.Status.OK.getName());
                            } else if(mFrameBufferQueue != null && acceptedFrameId >= 0) {
                                LogService.d(TAG, "add frame to queue " + mTotalFramesCaptured);
                                mFrameBufferQueue.offer(new Pair<byte[], Pair<Long, Long>>(bytereader.clone(), new Pair<Long, Long>(mTotalFramesCaptured, currentTimeStamp)));
                                mTotalFramesCaptured++;
                            }
                            break;
                        }
                        case STRING_BUFFER_CAPTURE: {
                            // addFrame if we're doing a fetch frame, or if the buffer is not full
                            int stopCameraCount = _mCaptureSettings.getBufferFrames() -1 + rawBufferCacheSize;
                            if (mFrameBufferQueue != null && acceptedFrameId >= 0) {
                                if ( stopCameraCount < mTotalFramesCaptured ) {
                                    stopStreamSync();
                                    setProject(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                    setFlood(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                    closeSync();
                                } else {
                                    LogService.d(TAG, "add frame to queue " + mTotalFramesCaptured);
                                    mFrameBufferQueue.offer(new Pair<byte[], Pair<Long, Long>>(bytereader.clone(), new Pair<Long, Long>(mTotalFramesCaptured, currentTimeStamp)));
                                    mTotalFramesCaptured++;
                                }
                            }
                        }
                        break;
                        case STRING_RECALIBRATION: {
                            if (acceptedFrameId >= 0) {
                                stopStreamSync();
                                setProject(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                setFlood(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                closeSync();
                                new Thread(new Runnable() {
                                    @Override
                                    public void run() {
                                        mDepthCameraStreamListener.onFrameNative(Arrays.copyOfRange(bytereader, COLOR_SIZE, COLOR_SIZE + IR_SIZE *2),
                                                mTotalFramesStreamed - skipFrameNum, currentTimeStamp, RESOLUTION_8M.ordinal(), false);
                                    }
                                }).start();
                            }
                            break;
                        }
                    }
                }
                image.close();
            } catch (Exception e){
                LogService.e(TAG, " exception !!!" + e.toString());
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
                image.close();
            }

        }

    };

    private Runnable processNative = new Runnable () {
        @Override
        public void run() {
            while (mNativeThreadControl) {
                Pair<byte[], Pair<Long,Long>> framedata = mNativeQueue.poll();

                if (framedata != null && framedata.first != null) {
                    boolean islock = false;
                    try {
                        islock = mCaptureLock.tryAcquire(5, TimeUnit.MILLISECONDS);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                        LogService.logStackTrace(TAG, e.getStackTrace());
                    }
                    LogService.v(TAG,"mTotalFramesStreamed = " + framedata.second.first + " , timestamp =  " + framedata.second.second);
                    /* calibration tool uses persis to control , will fix later */
                    int decodeRes = RESOLUTION_2M.ordinal();
                    if(COLOR_HEIGHT == 3264 && COLOR_WIDTH == 2448)
                        decodeRes = RESOLUTION_8M.ordinal();

                    mDepthCameraStreamListener.onFrameNative(framedata.first, framedata.second.first, framedata.second.second,decodeRes, mIsCapture);


                    if(mIsCapture) mIsCapture = false;
                    if(islock) mCaptureLock.release();
                }
            }
        }
    };

    private byte[] processSyncFrame(final byte[] newBytedata, long currentTimeStamp, long currentFrameID) {
        if(rawBufferCache.size() < rawBufferCacheSize ) return newBytedata;
        byte[] currentBytedata = newBytedata;

        try {
            int curbufferIndex = rawBufferCacheSize - 2;
            currentBytedata = rawBufferCache.get(curbufferIndex).clone();

            // get cached buffer time stamps
            for (int i = 0; i < rawBufferCacheSize ; ++i) {
                timeStampMsCache[i] = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                        Arrays.copyOfRange(rawBufferCache.get(i), COLOR_SIZE + IR_SIZE * 2, COLOR_SIZE + IR_SIZE * 2 + 8)))).getLong();
                timeStampLsCache[i] = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                        Arrays.copyOfRange(rawBufferCache.get(i), COLOR_SIZE + IR_SIZE * 2 + 8, COLOR_SIZE + IR_SIZE * 2 + 16)))).getLong();
                timeStampRsCache[i] = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                        Arrays.copyOfRange(rawBufferCache.get(i), COLOR_SIZE + IR_SIZE * 2 + 16, COLOR_SIZE + IR_SIZE * 2 + 24)))).getLong();
            }

            // log time stamps for debugs
            for (int i = 0; i < rawBufferCacheSize; ++i) {
                LogService.v(TAG, "sync raw buffer - Cached buffer time stamps (M, L, R)"
                        + "[" + i + "]: "
                        + timeStampMsCache[i] / NSTOMS + " "
                        + timeStampLsCache[i] / NSTOMS + " "
                        + timeStampRsCache[i] / NSTOMS);
            }

            // sync M and R base on L
            long targetTimeStampL = timeStampLsCache[curbufferIndex];
            int minDiffIdxR = curbufferIndex, minDiffIdxM = curbufferIndex;
            long minDiffLR = Math.abs(timeStampRsCache[curbufferIndex] - targetTimeStampL);
            long minDiffLM = Math.abs(timeStampMsCache[curbufferIndex] - targetTimeStampL);
            long minTimeStampL = targetTimeStampL;
            int minTimeStampLIdx = 0;

            // ignore curbufferIndex index
            for (int i = 0; i < rawBufferCacheSize; ++i) {
                if( i == curbufferIndex) continue;
                if (Math.abs(timeStampRsCache[i] - targetTimeStampL) < minDiffLR) {
                    minDiffIdxR = i;
                    minDiffLR = Math.abs(timeStampRsCache[i] - targetTimeStampL);
                }

                if (Math.abs(timeStampMsCache[i] - targetTimeStampL) < minDiffLM) {
                    minDiffIdxM = i;
                    minDiffLM = Math.abs(timeStampMsCache[i] - targetTimeStampL);
                }
            }
            LogService.d(TAG, "sync raw buffer - Selected buffer time stamps (M, L, R): "
                    + "[" + minDiffIdxM + "]" + timeStampMsCache[minDiffIdxM] / NSTOMS
                    + "[" + (targetTimeStampL / NSTOMS - timeStampMsCache[minDiffIdxM] / NSTOMS) + "]" + " "
                    + targetTimeStampL / NSTOMS + " "
                    + "[" + minDiffIdxR + "]" + timeStampRsCache[minDiffIdxR] / NSTOMS
                    + "[" + (targetTimeStampL / NSTOMS - timeStampRsCache[minDiffIdxR] / NSTOMS) + "]"
            );

            // cache newest buffer
            // rawBufferCache.set(minTimeStampLIdx, currentBytedata.clone());

            // update currentBytedata
            if (minDiffIdxR != curbufferIndex)  // update image R if need
            {
                int dataPos = COLOR_SIZE + IR_SIZE;
                System.arraycopy(rawBufferCache.get(minDiffIdxR), dataPos, currentBytedata, dataPos, IR_SIZE);
                int timeStampPos = COLOR_SIZE + IR_SIZE * 2 + TIMESTAMP_SIZE * 2;
                System.arraycopy(rawBufferCache.get(minDiffIdxR), timeStampPos, currentBytedata, timeStampPos, TIMESTAMP_SIZE);
            }

            if (minDiffIdxM != curbufferIndex)  // update image M if need
            {
                int dataPos = 0;
                System.arraycopy(rawBufferCache.get(minDiffIdxM), dataPos, currentBytedata, dataPos, COLOR_SIZE);
                int timeStampPos = COLOR_SIZE + IR_SIZE * 2;
                System.arraycopy(rawBufferCache.get(minDiffIdxM), timeStampPos, currentBytedata, timeStampPos, TIMESTAMP_SIZE);
            }

            // update cache
            rawBufferCache.remove(0);
            rawBufferCache.add(newBytedata.clone());
            LogService.d(TAG,"rawBufferCache size : " + rawBufferCache.size());

        } catch (Exception e) {
            LogService.w(TAG, "Failed to sync LMR at frame buffer level, " + e.toString());
            LogService.d(TAG,"rawBufferCache size : " + rawBufferCache.size());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return currentBytedata;
        }

        long timeStampM = 0L, timeStampL = 0L, timeStampR = 0L;

        try {
            /* c timestamp is big endian, java byte array is small endian, so need to reverse it */
            timeStampM = ByteBuffer.wrap((FormattingService.byteArrayReverse(Arrays.copyOfRange(currentBytedata, COLOR_SIZE + IR_SIZE * 2, COLOR_SIZE + IR_SIZE * 2 + 8)))).getLong();
            timeStampL = ByteBuffer.wrap((FormattingService.byteArrayReverse(Arrays.copyOfRange(currentBytedata, COLOR_SIZE + IR_SIZE * 2 + 8, COLOR_SIZE + IR_SIZE * 2 + 16)))).getLong();
            timeStampR = ByteBuffer.wrap((FormattingService.byteArrayReverse(Arrays.copyOfRange(currentBytedata, COLOR_SIZE + IR_SIZE * 2 + 16, COLOR_SIZE + IR_SIZE * 2 + 24)))).getLong();
            LogService.i(TAG, "current use frame [M,L,R] : [" + timeStampM + "," + timeStampL + "," + timeStampR + "]");
            LogService.i(TAG, "current frame diff [M-L,L-R] : [" + (timeStampM - timeStampL) / NSTOMS + "," + (timeStampL - timeStampR) / NSTOMS + "]");

        } catch (Exception e) {
            LogService.w(TAG, " might not have timestamp info");
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
            return currentBytedata;
        }

        return currentBytedata;
    }

    private Runnable processArc1 = new Runnable () {
        @Override
        public void run() {
            while (mFrameBufferThreadControl) {
                Pair<byte[], Pair<Long,Long>> framedata = mFrameBufferQueue.poll();

                if (framedata != null && framedata.first != null) {

                    final byte[] currentBytedata = framedata.first;
                    final long currentTimeStamp = framedata.second.second;
                    final long currentFrameID = framedata.second.first;

                    byte[] syncedFrame = processSyncFrame(currentBytedata.clone(), currentTimeStamp, currentFrameID);

                    mDepthCameraStreamListener.onFrameHost(_mCaptureSettings,syncedFrame.clone(), currentFrameID);

                    /* send frame to native do find LandMarks */
                    if(currentFrameID <= 5)
                        mDepthCameraStreamListener.onFrameNative(syncedFrame.clone(), currentFrameID, currentTimeStamp, RESOLUTION_2M.ordinal(), false);

                    /* send check frame enough or not */
                    if (currentFrameID == _mCaptureSettings.getDepthFrames() -1) {
                        if (mFrameBufferHandlerThread != null) {
                            mFrameBufferHandlerThread.quitSafely();
                            mFrameBufferHandlerThread = null;
                        }
                        mFrameBufferThreadControl = false;

                        rawBufferCache.clear();
                        isRawBufferCacheInit = false;
                        timeStampMsCache = timeStampLsCache = timeStampRsCache = null;
                    }
                }
            }
        }
    };

    private Runnable processArcX = new Runnable () {
        @Override
        public void run() {

            while (mFrameBufferThreadControl) {
                Pair<byte[], Pair<Long,Long>> framedata = mFrameBufferQueue.poll();

                if (framedata != null && framedata.first != null) {
                    byte[] currentBytedata = framedata.first;
                    final long currentTimeStamp = framedata.second.second;
                    final long currentFrameID = framedata.second.first;

                    byte[] syncedFrame = processSyncFrame(currentBytedata.clone(), currentTimeStamp, currentFrameID);

                    String action = _mCaptureSettings.getRequest().toLowerCase();
                    int stopFrameID = _mCaptureSettings.getBufferFrames() -1;
                    switch (action) {
                        case STRING_BUFFER_CAPTURE : {
                            LogService.d(TAG, "add frame : "+ currentFrameID);
                            mDepthCameraStreamListener.onFrameHost(_mCaptureSettings,  syncedFrame.clone(), currentFrameID);

                            // stop process thread when done
                            if (currentFrameID == stopFrameID) {
                                if (mFrameBufferHandlerThread != null) {
                                    mFrameBufferHandlerThread.quitSafely();
                                    mFrameBufferHandlerThread = null;
                                }
                                mFrameBufferThreadControl = false;
                            }
                            break;
                        }
                        case STRING_STREAM_DEPTH : {
                            LogService.d(TAG, "start process frame id " + currentFrameID);
                            mDepthCameraStreamListener.onFrameHost(_mCaptureSettings, syncedFrame.clone(), currentFrameID);//color
                            mDepthCameraStreamListener.onFrameNative(Arrays.copyOfRange(syncedFrame, COLOR_SIZE, COLOR_SIZE + IR_SIZE * 2 + TIMESTAMP_SIZE * 3).clone(), currentFrameID, currentTimeStamp, RESOLUTION_8M.ordinal(), false);

                            // stop process thread when done
                            if (currentFrameID == _mCaptureSettings.getStreamDepthFrames() -1) {
                                if (mFrameBufferHandlerThread != null) {
                                    mFrameBufferHandlerThread.quitSafely();
                                    mFrameBufferHandlerThread = null;
                                }
                                mFrameBufferThreadControl = false;
                            }
                            break;
                        }
                        case STRING_CANCEL_PROCESS : {
                            // self terminate
                            if (mFrameBufferHandlerThread != null) {
                                mFrameBufferHandlerThread.quitSafely();
                                mFrameBufferHandlerThread = null;
                            }
                            mFrameBufferThreadControl = false;
                            break;
                        }
                        default:
                            LogService.w(TAG,"not supported action : " + action);
                    }
                }
            }
        }
    };

}
