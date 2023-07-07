package com.bellus3d.android.arc.b3d4client.camera;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.icu.text.SimpleDateFormat;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.util.Range;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.Surface;
import android.view.TextureView;
import android.view.WindowManager;

import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.CaptureEvent.CAPTURE_SINGLE;

/* DepthCamera_Files will use camera API2, the real frame/Images will add from native side */
public class DepthCamera_Files extends DepthCameraImpl {

    Context _context;

    private static final String TAG = "DepthCamera_Files";

    private boolean captureOn = false;

    private boolean mUseFrontCamera = false;
    public static final String CAMERA_FRONT = "1";
    public static final String CAMERA_BACK = "0";
    private String mRequestEvent = null;
    private String mRequestId = null;
    private Integer mLensFacing = -1;
    private int mSensorOrientation = 0;
    private int mSurfaceRotation = 0;
    private int mFormat = 0;
    private Size mSize;
    private Range<Integer> mFpsRange;

    private String mCameraId;
    private CameraCaptureSession mCaptureSession;
    private CameraDevice mCameraDevice;

    private HandlerThread backgroundThread;
    private Handler backgroundHandler;
    private ImageReader imageReader;

    private long startProcessTimestamp = 0;
    private long startCaptureTimestamp = 0;
    private long capturedFrames = 0;
    private long capturedBytes = 0;
    private long totalCaptureMilliseconds = 0;
    private long totalProcessMilliseconds = 0;

    private B3DCaptureSettings _mCaptureSettings;

    private DepthCameraStreamListener _cameraFrameListener;

    private File Flood = new File("/sys/flood/brightness");
    private File Projector = new File("/sys/projector/brightness");

    private Semaphore mCameraOpenCloseLock = new Semaphore(1);

    private static final SparseIntArray ORIENTATIONS = new SparseIntArray(4);
    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }
    
    @Override
    public void setCameraID(String cameraID) {
        mCameraId = cameraID;
        mUseFrontCamera = mCameraId.equals(CAMERA_FRONT);
    }

    public B3DCaptureSettings.Format getFormat() {
        return getFormatFromNative(mFormat);
    }
    public Size getSize() {
        return mSize;
    }
    public Range<Integer> getFpsRange() {
        return mFpsRange;
    }
    public long getCapturedFrames() { return capturedFrames; };
    public long getCapturedBytes() { return capturedBytes; };
    public long getCaptureMilliseconds() { return totalCaptureMilliseconds; };
    public long getProcessMilliseconds() { return totalProcessMilliseconds; }


    @Override
    boolean setFlood(B3DDepthCamera.Control ctrl, String value) {
        if(Flood.exists()) {
            try {
                FileWriter fileWriter = new FileWriter(Flood);
                String level = ctrl.equals(B3DDepthCamera.Control.CONTROL_ON) ? "255" : "0";
                fileWriter.write(level);
                fileWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
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
                String level = ctrl.equals(B3DDepthCamera.Control.CONTROL_ON) ? "255" : "0";
                fileWriter.write(level);
                fileWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            LogService.e(TAG, "Projector sys node not exist");
            return false;
        }
        return true;
    }

    @Override
    DepthCameraError setFps(Range<Integer> fps) {
        return null;
    }

    @Override
    float getFps() {
        return 0;
    }

    @Override
    void setCaptureSettings(B3DCaptureSettings mCaptureSetting) {
        _mCaptureSettings = mCaptureSetting;
    }

    public String getRequestEvent() {
        return mRequestEvent;
    }
    public String getRequestId() {
        return mRequestId;
    }
    /*   do we still need these functions ? */

    DepthCamera_Files(final Context context, B3DDepthCamera.DeviceType _DepthCamType) {
        super(_DepthCamType);
        LogService.d(TAG, "Creating FILES/Camera2 DepthCamera");
        DepthCameraJNI(B3DDepthCamera.DeviceType.DEVICE_FILES.ordinal());
        _context = context;

        ArrayList<B3DCaptureSettings.CameraSource> sources = new ArrayList<B3DCaptureSettings.CameraSource>();
        sources.add(B3DCaptureSettings.CameraSource.COLOR);
        _mCaptureSettings.setSupportedSources(sources);

        ArrayList<B3DCaptureSettings.Format> formats = new ArrayList<B3DCaptureSettings.Format>();
        formats.add(B3DCaptureSettings.Format.JPEG);
        _mCaptureSettings.setSupportedFormats(formats);

        // Default settings
        _mCaptureSettings.set(B3DCaptureSettings.CameraSource.COLOR, B3DCaptureSettings.Format.JPEG, 320, 240, 20.0);
    }

    @Override
    B3D4ClientError setPreviewTexture(TextureView previewTexture, TextureView testTextureView) {
        return null;
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    //open when b3dservice start
    B3D4ClientError openSync() {

        setUpCameraOutputs();
        CameraManager manager = (CameraManager) _context.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            startBackgroundThread();
            manager.openCamera(mCameraId, mStateCallback, backgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }

        return null;
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    //close when b3dservice destroy
    B3D4ClientError closeSync() {
        try {
            mCameraOpenCloseLock.acquire();
            if (null != mCaptureSession) {
                mCaptureSession.close();
                mCaptureSession = null;
            }
            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }
            if (null != imageReader) {
                imageReader.close();
                imageReader = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
            stopBackgroundThread();
        }

        return null;
    }

    @Override
    //CMD_START_STREAM
    B3D4ClientError startStreamSync() {
        captureOn = true;
        totalProcessMilliseconds = 0;
        totalCaptureMilliseconds = 0;
        capturedFrames = 0;
        capturedBytes = 0;
        return null;
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    //CMD_STOP_STREAM
    B3D4ClientError stopStreamSync() {
        try {
            captureOn = false;
            mCaptureSession.abortCaptures();
            //mCaptureSession.stopRepeating();
        } catch (Exception e) {
            LogService.d(TAG, "stopStreamSync: "+e.toString());
        }
        return null;
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    //CMD_SNAP_IMAGE
    B3D4ClientError setCaptureEvent(B3DCaptureSettings settings, B3DDepthCamera.CaptureEvent CEvent) {
        boolean is_single = (CEvent == CAPTURE_SINGLE);

        try {
            if (null == mCameraDevice) {
                return new B3D4ClientError(B3D4ClientError.B3D_ERROR_CATEGORY.B3D_OK.getValue());
            }


            mRequestEvent = _mCaptureSettings.getRequest();
            mRequestId = _mCaptureSettings.getRequestID();

            // This is the CaptureRequest.Builder that we use to take a picture.
            int videoTemplate = CameraDevice.TEMPLATE_STILL_CAPTURE; // 10 FPS

            int template = is_single ? CameraDevice.TEMPLATE_STILL_CAPTURE : videoTemplate;
            final CaptureRequest.Builder captureBuilder = mCameraDevice.createCaptureRequest(template);
            captureBuilder.addTarget(imageReader.getSurface());

            if (is_single) {
                //captureBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            } else {
                captureBuilder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_OFF);

                //captureBuilder.set(CaptureRequest.SENSOR_EXPOSURE_TIME, Long.valueOf("1000000"));
                //captureBuilder.set(CaptureRequest.SENSOR_FRAME_DURATION, Long.valueOf("1000000"));
                captureBuilder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, mFpsRange);
            }

            // Orient camera capture
            WindowManager windowManager = (WindowManager) _context.getSystemService(Context.WINDOW_SERVICE);
            int orientation = windowManager.getDefaultDisplay().getRotation();
            mSurfaceRotation = ORIENTATIONS.get(orientation);
            int imageOrientation = getImageOrientation(orientation);
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, imageOrientation);

            // Captuere callbacks
            CameraCaptureSession.CaptureCallback CaptureCallback = new CameraCaptureSession.CaptureCallback() {

                @Override
                public void onCaptureStarted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, long timestamp, long frameNumber) {
                    startCaptureTimestamp = System.currentTimeMillis();
                    LogService.d(TAG, "onCaptureStarted: "+mRequestId+", frame "+frameNumber);
                }

                @Override
                public void onCaptureCompleted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull TotalCaptureResult result) {
                    // Note: Capture includes image processing time
                    final long now = System.currentTimeMillis();
                    long ms = now - startProcessTimestamp;
                    totalProcessMilliseconds += ms;
                    LogService.d(TAG, "onCaptureCompleted: "+now+"-"+startProcessTimestamp+"="+ms+", "+totalProcessMilliseconds);
                }
            };

            if(is_single) {
                mCaptureSession.stopRepeating();
                mCaptureSession.capture(captureBuilder.build(), CaptureCallback, null);
            } else {
                mCaptureSession.setRepeatingRequest(captureBuilder.build(), CaptureCallback, null);
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        return null;
    }

    @Override
    void registerStreamListener(DepthCameraStreamListener listener) {
        _cameraFrameListener = listener;
    }

    @Override
    void unregisterStreamListener(DepthCameraStreamListener listener) {
        _cameraFrameListener = null;
    }

    @Override
    void setPreviewTarget(TextureView viewL, TextureView viewR, TextureView viewM) {

    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private void setUpCameraOutputs() {
        B3DCaptureSettings.CameraSource source = _mCaptureSettings.getSource();
        B3DCaptureSettings.Format format = _mCaptureSettings.getFormat();
        int width = _mCaptureSettings.getWidth();
        int height = _mCaptureSettings.getHeight();
        double FPS = _mCaptureSettings.getFPS();

        // Get format
        mFormat = getNativeFormat(format);

        CameraManager manager = (CameraManager) _context.getSystemService(Context.CAMERA_SERVICE);
        try {
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);

                // Check whether we want a front facing camera or not
                mSensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                mLensFacing = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (mLensFacing != null) {
                    if (mUseFrontCamera ^ mLensFacing == CameraCharacteristics.LENS_FACING_FRONT) continue;
                }

                StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                if (map == null) continue;
                LogService.v(TAG, "Stream options: "+map.toString());

                // We default to largest size
                Size[] sizes = map.getOutputSizes(mFormat);
                Size largest = Collections.max(Arrays.asList(sizes), new CompareSizesByArea());

                if (width == 0 && height == 0) {
                    mSize = largest;
                } else {
                    // Get the closest match
                    // Probably a better way to do this
                    int area = width*height;
                    int diff = largest.getWidth()*largest.getHeight();
                    Size match = largest;
                    for (int i=0; i<sizes.length; i++) {
                        LogService.d(TAG, sizes[i].toString());
                        Size size = sizes[i];
                        int a = size.getWidth()*size.getHeight();
                        int d = Math.abs(area-a);
                        if (d < diff) {
                            diff = d;
                            match = size;
                        }
                    }
                    mSize = match;
                    LogService.i(TAG, "Using frame size: "+match.toString());
                }
                mCameraId = cameraId;

                // For still image captures, we use the largest available size.
                int w = mSize.getWidth();
                int h = mSize.getHeight();

                imageReader = ImageReader.newInstance(w, h, mFormat, /*maxImages*/2);
                imageReader.setOnImageAvailableListener(mOnImageAvailableListener, backgroundHandler);

                // Get fastest FPS
                Range<Integer> fps[] = characteristics.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
                int index = fps.length - 1;
                int highMax = 0;
                int lowMax = 0;
                for (int i=0; i<fps.length; i++) {
                    LogService.i(TAG, "Supported FPS: "+fps[i].toString());
                    Range<Integer> range = fps[i];
                    Integer low = range.getLower();
                    Integer high = range.getUpper();
                    if (high > highMax) {
                        highMax = high;
                        lowMax = 0;
                        index = i;
                    } else if (high == highMax) {
                        if (low > lowMax) {
                            lowMax = low;
                            index = i;
                        }
                    }
                }
                mFpsRange = fps[index];
                LogService.d(TAG, "Using FPS range: "+mFpsRange.toString());

                return;
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            // Currently an NPE is thrown when the Camera2API is used but not supported on the
            // device this code runs.
            e.printStackTrace();
        }
    }

    private boolean hasCapability(int[] capabilities, int capability) {
        for (int c : capabilities) {
            if (c == capability) return true;
        }
        return false;
    }

    private int getNativeFormat(B3DCaptureSettings.Format format) {
        switch (format) {
            default:
            case JPEG: {
                return ImageFormat.JPEG;
            }
        }
    }

    private B3DCaptureSettings.Format getFormatFromNative(int format) {
        switch (format) {
            default:
            case ImageFormat.JPEG: {
                return B3DCaptureSettings.Format.JPEG;
            }
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            // This method is called when the camera is opened.  We start camera preview here.
            mCameraOpenCloseLock.release();
            mCameraDevice = cameraDevice;
            createCameraCaptureSession();
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
        }

    };

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private final ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {

        @Override
        public void onImageAvailable(ImageReader reader) {
            LogService.d(TAG, "onImageAvailable");
            if (!captureOn) return;

            // Note: this is called after the frame is captured, but before it's processed
            startProcessTimestamp = System.currentTimeMillis();
            long ms = startProcessTimestamp - startCaptureTimestamp;
            LogService.d(TAG, "onImageAvailable: "+startProcessTimestamp+"-"+startCaptureTimestamp+"="+ms);
            totalCaptureMilliseconds += ms;
            capturedFrames++;

            try (Image image = reader.acquireNextImage()) {
                Image.Plane plane = image.getPlanes()[0];
                ByteBuffer buffer = plane.getBuffer();

                final byte[] byteData = new byte[buffer.remaining()];
                buffer.get(byteData);
                //LogService.i(TAG, "onImageAvailable frame size: " + byteData.length);
                capturedBytes += byteData.length;

                backgroundHandler.post(
                        new Runnable() {
                            @Override
                            public void run() {
                                _cameraFrameListener.onFrameHost(_mCaptureSettings, byteData,capturedFrames);
                            }
                        }
                );
            }
        }

    };

    private void startBackgroundThread() {
        backgroundThread = new HandlerThread("CameraBackground");
        backgroundThread.start();
        backgroundHandler = new Handler(backgroundThread.getLooper());
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private void stopBackgroundThread() {
        if(backgroundThread != null) {
            //https://stackoverflow.com/questions/8907626/is-this-a-prefect-way-to-stop-handlerthread
            backgroundThread.quitSafely();
            try {
                backgroundThread.join(1000);
            } catch (InterruptedException e) {
                LogService.e(TAG,e.toString());
            }
            backgroundThread = null;
            backgroundHandler = null;
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private void createCameraCaptureSession() {
        try {

            // Here, we create a CameraCaptureSession for camera preview.
            mCameraDevice.createCaptureSession(Arrays.asList(imageReader.getSurface()),
                    new CameraCaptureSession.StateCallback() {

                        @Override
                        public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
                            // The camera is already closed
                            if (null == mCameraDevice) {
                                return;
                            }

                            // When the session is ready, we start displaying the preview.
                            mCaptureSession = cameraCaptureSession;
                        }

                        @Override
                        public void onConfigureFailed(@NonNull CameraCaptureSession cameraCaptureSession) {
                            LogService.d(TAG, "Configuration Failed");
                        }
                    }, null
            );
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private int getImageOrientation(int deviceOrientation) {
        if (deviceOrientation == android.view.OrientationEventListener.ORIENTATION_UNKNOWN) return 0;

        ///* Google's example
        // Round device orientation to a multiple of 90
        deviceOrientation = (deviceOrientation + 45) / 90 * 90;

        // Reverse device orientation for front-facing cameras
        boolean facingFront = mLensFacing == CameraCharacteristics.LENS_FACING_FRONT;
        if (facingFront) deviceOrientation = -deviceOrientation;

        // Calculate desired JPEG orientation relative to camera orientation to make
        // the image upright relative to the device orientation
        int imageOrientation = (mSensorOrientation + deviceOrientation + 360) % 360;
        return imageOrientation;
    }

    private static class ImageSaver implements Runnable {

        /**
         * The JPEG image
         */
        private final Image mImage;
        /**
         * The file we save the image into.
         */
        private final File mFile;

        public ImageSaver(Image image, File file) {
            mImage = image;
            mFile = file;
        }

        @Override
        @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
        public void run() {
            //LogService.i(TAG, "ImageSaver");
            long start = System.currentTimeMillis();
            ByteBuffer buffer = mImage.getPlanes()[0].getBuffer();
            byte[] bytes = new byte[buffer.remaining()];
            buffer.get(bytes);
            FileOutputStream output = null;
            try {
                output = new FileOutputStream(mFile);
                output.write(bytes);
                long ms = System.currentTimeMillis() - start;
                LogService.d(TAG, "ImageSaver: "+ms+"ms, "+mFile.toString());
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                mImage.close();
                if (null != output) {
                    try {
                        output.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() - (long) rhs.getWidth() * rhs.getHeight());
        }

    }

    @TargetApi(Build.VERSION_CODES.N)
    @RequiresApi(api = Build.VERSION_CODES.N)
    private File getOutputMediaFile() {
        // To be safe, you should check that the SDCard is mounted
        // using Environment.getExternalStorageState() before doing this.

        File mediaStorageDir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "Camera2Test");

        // This location works best if you want the created images to be shared
        // between applications and persist after your app has been uninstalled.

        // Create the storage directory if it does not exist
        if (!mediaStorageDir.exists()) {
            if (!mediaStorageDir.mkdirs()) {
                return null;
            }
        }

        // Create a media file name
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());

        File mediaFile;
        mediaFile = new File(mediaStorageDir.getPath() + File.separator + "IMG_" + timeStamp + ".jpg");

        return mediaFile;
    }
}
