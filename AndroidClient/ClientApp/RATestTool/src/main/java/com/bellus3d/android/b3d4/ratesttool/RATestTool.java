package com.bellus3d.android.b3d4.ratesttool;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.TextureView;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;

import com.bellus3d.android.arc.b3d4client.ConfigService.CameraConfigService;
import com.bellus3d.android.arc.b3d4client.DiskService;
import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.camera.B3DCaptureSettings;
import com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraStreamListener;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.CaptureEvent.*;
import static com.bellus3d.android.b3d4.ratesttool.RAUtils.*;
import static com.bellus3d.android.b3d4.ratesttool.RAUtils.LED_TYPE.*;

public class RATestTool extends AppCompatActivity {

    private static String TAG = "RATestTool";
    File Flood = new File("/sys/flood/brightness");
    File Projector = new File("/sys/projector/brightness");
    static B3DDepthCamera.Control FloodStatus = B3DDepthCamera.Control.CONTROL_OFF;
    static B3DDepthCamera.Control ProjectorStatus = B3DDepthCamera.Control.CONTROL_OFF;

    Button mCaptureBtn, mCamOpenBtn, mCamCloseBtn, mFloodBtn, mProjectorBtn;

    CheckBox ResSwitchchk;

    private TextureView mCamLTexture, mCamRTexture, mCamMTexture;

    B3DDepthCamera depthCamera;

    PermissionService permissionService;

    HandlerThread mSaveRecoredHandlerThread;

    Handler mSaveRecoredHandler;

    private long StartTime, currentlTime;

    private long mStreamCounter = 0;

    private FileWriter fw;

    private String RAResultPath = "/sdcard/RATest/RATest.txt";

    String[] permissions = {
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.ACCESS_WIFI_STATE,
            Manifest.permission.CHANGE_WIFI_STATE,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.CAMERA
    };

    protected void setupUICompoment() {

        mCamLTexture = (TextureView) findViewById(R.id.camLTexture);
        mCamRTexture = (TextureView) findViewById(R.id.camRTexture);
        mCamMTexture = (TextureView) findViewById(R.id.camMTexture);

        mCamLTexture.setSurfaceTextureListener(mSurfaceTextureListener);
        mCamRTexture.setSurfaceTextureListener(mSurfaceTextureListener);
        mCamMTexture.setSurfaceTextureListener(mSurfaceTextureListener);

        depthCamera = new B3DDepthCamera(this,B3DDepthCamera.DeviceType.DEVICE_B3D4);
        depthCamera.registerStreamListener(mDepthCameraStreamListener);
        //depthCamera.setPreviewTarget(mCamLTexture,mCamRTexture,mCamMTexture);

        /* onSurfaceTextureAvailable does not get called if it is already available */
        if (mCamLTexture.isAvailable() && mCamRTexture.isAvailable() && mCamMTexture.isAvailable()) {
            depthCamera.setPreviewTarget(mCamLTexture, mCamRTexture, mCamMTexture);
            mCamOpenBtn.setClickable(true);
            mCamCloseBtn.setClickable(true);
            mCaptureBtn.setClickable(true);
        }

        mStreamCounter = 0;

        mCamOpenBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    public void run() {
                        mStreamCounter = 0;
                        ResSwitchchk.setClickable(false);
                        if(ResSwitchchk.isChecked()) {
                            CameraConfigService.setCamera17FPS(5,(long)(3.5*1000));
                            depthCamera.setCaptureEvent(null,CAPTURE_BUFFER_8M);
                            B3D4ClientError openError = depthCamera.openSync();
                            LogService.d(TAG, "depthCamera open " + openError.getErrorMessage(openError.getErrorCode()));
                        } else {
                            CameraConfigService.setCamera17FPS(20,(long)(3.5*1000));
                            depthCamera.setCaptureEvent(null,CAPTURE_BUFFER_2M);
                            B3D4ClientError openError = depthCamera.openSync();
                            LogService.d(TAG, "depthCamera open " + openError.getErrorMessage(openError.getErrorCode()));
                        }
                    }
                });

                depthCamera.setPreviewTexture(mCamMTexture,null);

                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    public void run() {
                        B3D4ClientError openError = depthCamera.startStreamSync();
                        LogService.d(TAG,"depthCamera startStreamSync " + openError.getErrorMessage(openError.getErrorCode()));
                    }
                });
            }
        });

        mCamCloseBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                depthCamera.stopStreamSync();
                depthCamera.closeSync();
                ResSwitchchk.setClickable(true);
            }
        });

        mCaptureBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                depthCamera.setCaptureEvent(null,CAPTURE_SINGLE);
            }
        });

        mFloodBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                B3DDepthCamera.Control ctrl = B3DDepthCamera.Control.CONTROL_OFF;
                String level = "0";
                if(FloodStatus.equals(B3DDepthCamera.Control.CONTROL_ON)) {
                    FloodStatus = B3DDepthCamera.Control.CONTROL_OFF;
                    ctrl = B3DDepthCamera.Control.CONTROL_OFF;
                    level = "0";
                } else {
                    FloodStatus = B3DDepthCamera.Control.CONTROL_ON;
                    ctrl = B3DDepthCamera.Control.CONTROL_ON;
                    level = "255";
                }
                depthCamera.setFlood(ctrl,level);
            }
        });

        mProjectorBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                B3DDepthCamera.Control ctrl = B3DDepthCamera.Control.CONTROL_OFF;
                String level = "0";
                if(ProjectorStatus.equals(B3DDepthCamera.Control.CONTROL_ON)) {
                    ProjectorStatus = B3DDepthCamera.Control.CONTROL_OFF;
                    ctrl = B3DDepthCamera.Control.CONTROL_OFF;
                    level = "0";
                } else {
                    ProjectorStatus = B3DDepthCamera.Control.CONTROL_ON;
                    ctrl = B3DDepthCamera.Control.CONTROL_ON;
                    level = "127";
                }
                depthCamera.setProject(ctrl,level);
            }
        });

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ratest_tool);

        mCaptureBtn     = (Button) findViewById(R.id.CaptureButton);
        mCamOpenBtn   = (Button) findViewById(R.id.CamOpenButton);
        mCamCloseBtn   = (Button) findViewById(R.id.CamCloseButton);
        mFloodBtn       = (Button) findViewById(R.id.FloodButton);
        mProjectorBtn    = (Button) findViewById(R.id.ProjectorButton);
        mCamOpenBtn.setClickable(false);
        mCamCloseBtn.setClickable(false);
        mCaptureBtn.setClickable(false);

        ResSwitchchk    = (CheckBox) findViewById(R.id.ResSwitchchk);

        ResSwitchchk.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                if (((CheckBox) v).isChecked()) {
                    ResSwitchchk.setText("8M");
                } else
                    ResSwitchchk.setText("2M");
            }
        });

        permissionService = new PermissionService(this,permissions);
        if(permissionService.checkAllPermissionsEnabled()) {
            setupUICompoment();
        } else {
            permissionService.requestMultiplePermissions();
        }

        Thread.setDefaultUncaughtExceptionHandler(
                new Thread.UncaughtExceptionHandler() {
                    @Override
                    public void uncaughtException (Thread thread, Throwable e) {
                        LogService.e(TAG,e.getMessage());
                        handleUncaughtException (thread, e);
                    }
                });
    }

    private void handleUncaughtException (Thread thread, Throwable e) {
        setLED(LED_GREEN, LED_CONTROL.LED_OFF);
        setLED(LED_RED, LED_CONTROL.LED_ON);
        ProjectorStatus = B3DDepthCamera.Control.CONTROL_OFF;
        FloodStatus = B3DDepthCamera.Control.CONTROL_OFF;
        depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF,"0");
        depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_OFF,"0");
        Intent intent = new Intent (getApplicationContext(),RATestTool.class);
        startActivity(intent);
    }

    @Override
    public void onStop() {
        super.onStop();
        ProjectorStatus = B3DDepthCamera.Control.CONTROL_OFF;
        FloodStatus = B3DDepthCamera.Control.CONTROL_OFF;
        depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF,"0");
        depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_OFF,"0");
        setLED(LED_GREEN, LED_CONTROL.LED_OFF);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        setLED(LED_GREEN, LED_CONTROL.LED_OFF);
    }


    /* a listener that returns frame from java camera API */
    private final DepthCameraStreamListener mDepthCameraStreamListener = new DepthCameraStreamListener() {

        @Override
        public void onWarning(B3D4ClientError error, B3DCaptureSettings captureSetting) {

        }

        @Override
        public void onError(B3D4ClientError error, B3DCaptureSettings captureSetting) {

        }

        @Override
        public void onStatusHost(B3DCaptureSettings mCaptureSetting, String status) {

        }

        @Override
        public void onMsgHost(B3DCaptureSettings mCaptureSetting, String msg) {
        }

        @Override
        public void onNotificationHost(B3DCaptureSettings mCaptureSetting, long timeStampL, long timeStampR, long frameIndex) {

        }

        @Override
        public void onFrameHost(B3DCaptureSettings mCaptureSetting, byte[] byteData, long frameID) {

        }

        @Override
        public void onFrameNative(byte[] byteData, long mTotalFramesStreamed, long currentTimeStamp, int mResolution, boolean isCapture) {
            ++mStreamCounter;
            //depthCamera.SetFrameJNI(byteData,mTotalFramesStreamed,currentTimeStamp,mResolution,isCapture);
        }
    };

    private TextureView.SurfaceTextureListener mSurfaceTextureListener = new TextureView.SurfaceTextureListener() {
        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            if (mCamLTexture.isAvailable() && mCamRTexture.isAvailable() && mCamMTexture.isAvailable()) {
                LogService.e(TAG, "onSurfaceTextureAvailable setPreviewTarget");
                depthCamera.setPreviewTarget(mCamLTexture, mCamRTexture, mCamMTexture);
                mCamOpenBtn.setClickable(true);
                mCamCloseBtn.setClickable(true);
                //mCaptureBtn.setClickable(true);
                mCamOpenBtn.callOnClick();
                mProjectorBtn.callOnClick();
                ResSwitchchk.setChecked(false);
                //mFloodBtn.callOnClick();

                setLED(LED_GREEN, LED_CONTROL.LED_ON);

                mSaveRecoredHandlerThread = new HandlerThread("SaveRecoredHandler");
                mSaveRecoredHandlerThread.start();
                mSaveRecoredHandler = new Handler(mSaveRecoredHandlerThread.getLooper());
                mSaveRecoredHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            DiskService.createDirectory(new File("/sdcard/RATest/"));
                            String filename = CameraConfigService.get("ro.serialno","Unknown") + "_" + String.valueOf((int)(Math.random()*100000+1));
                            RAResultPath = "/sdcard/RATest/RATest_" + filename + ".txt";
                            fw = new FileWriter(RAResultPath);
                            StartTime = System.currentTimeMillis();
                            String result = "Start RATest : framecount : 0" + "\r\n";
                            fw.write(result);
                            fw.flush();
                            fw.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                            LogService.logStackTrace(TAG, e.getStackTrace());
                        }

                        while(true) {
                            try {
                                Thread.sleep(1000*1800); //30m to recored
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                                LogService.logStackTrace(TAG, e.getStackTrace());
                            }

                            try {
                                fw = new FileWriter(RAResultPath,true);
                                currentlTime = System.currentTimeMillis();
                                String result = "From RATest Start :  " + (currentlTime-StartTime)/1000 + " s  , frame count : " + mStreamCounter + "\r\n";
                                fw.write(result);
                                fw.flush();
                                fw.close();
                            } catch (IOException e) {
                                e.printStackTrace();
                                LogService.logStackTrace(TAG, e.getStackTrace());
                            }
                        }
                    }
                });

            }
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
            return false;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surface) {

        }
    };

    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case PermissionService.REQUEST_CODE_B3D4_PERMISSION: {
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    setupUICompoment();
                } else {
                }
                return;
            }
        }
    }
}
