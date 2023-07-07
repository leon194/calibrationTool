package com.bellus3d.android.calibrationtool;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.TextureView;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

import com.bellus3d.android.arc.b3d4client.ConfigService.CameraConfigService;
import com.bellus3d.android.arc.b3d4client.ConfigService.LocalConfigService;
import com.bellus3d.android.arc.b3d4client.DiskService;
import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.ProcessingService;
import com.bellus3d.android.arc.b3d4client.RecordingService;
import com.bellus3d.android.arc.b3d4client.camera.B3DCaptureSettings;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraStreamListener;

import java.io.File;
import java.util.List;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.ARC_APP_PATH;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.EXP_LINE_TIME_2M;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.EXP_LINE_TIME_8M;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.NSTOMS;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.OV8856_BINNING_FACTOR;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.CaptureEvent.*;


public class CalibrationTool extends AppCompatActivity{

    private static String TAG = "CalibrationTool";
    File Flood = new File("/sys/flood/brightness");
    File Projector = new File("/sys/projector/brightness");

    private WifiTestUtil wifiTestUtil = null;
    private WifiManager mWifiManager = null;
    private boolean mIs5GHzBandSupported = false;
    private String HostName = "Bellus3d";
    private int HostLevelThres = -50;
    int gainArray[];

    Button mCaptureBtn, mCamOpenBtn, mCamCloseBtn, mFloodBtn, mProjectorBtn;

    Button mWifiOpenButton, mWifiCloseButton;

    CheckBox ResSwitchchk;

    private TextView mWifiState = null;

    private TextView mFPS;

    private TextureView mCamLTexture, mCamRTexture, mCamMTexture;

    private EditText mExpRate, mWBRate;

    RecordingService _recordingService;

    ProcessingService _processingService;

    PermissionService _permissionService;

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
        mExpRate = (EditText) findViewById(R.id.exprate);
        mWBRate  = (EditText) findViewById(R.id.wbRate);

        mCamLTexture.setSurfaceTextureListener(mSurfaceTextureListener);
        mCamRTexture.setSurfaceTextureListener(mSurfaceTextureListener);
        mCamMTexture.setSurfaceTextureListener(mSurfaceTextureListener);

        _recordingService.initializeCamera(this);
        _recordingService.setPreviewTexture(mCamMTexture);
        _recordingService.registerStreamListener(mDepthCameraStreamListener);

        _processingService = new ProcessingService();

        /* onSurfaceTextureAvailable does not get called if it is already available */
        if (mCamLTexture.isAvailable() && mCamRTexture.isAvailable() && mCamMTexture.isAvailable()) {
            //depthCamera.setPreviewTarget(mCamLTexture, mCamRTexture, mCamMTexture);
            _recordingService.setPreviewTarget(mCamLTexture, mCamRTexture, mCamMTexture);
            mCamOpenBtn.setClickable(true);
            mCamCloseBtn.setClickable(true);
            mCaptureBtn.setClickable(true);
        }

        mCamOpenBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                gainArray = LocalConfigService.GetLastAEAWBValueJNI();
                float aeRatio = mExpRate.getText().toString().trim().length() > 0 ? Float.parseFloat(mExpRate.getText().toString()) : 0.0f;
                float awbRatio = mWBRate.getText().toString().trim().length() > 0 ? Float.parseFloat(mWBRate.getText().toString()) : 0.0f;

                int expline = gainArray[0];
                int expTime = (int) (gainArray[2] / NSTOMS);
                int target_expline = expline * EXP_LINE_TIME_8M / EXP_LINE_TIME_2M * OV8856_BINNING_FACTOR;
                int gain = gainArray[1];
                int gainRed = gainArray[3];
                int gainGreen = gainArray[4];
                int gainBlue = gainArray[5];
                LogService.i(TAG, "manual ctrl aeRatio : " + aeRatio + ", awbRatio : " + awbRatio);
                LogService.i(TAG, "manual ctrl set properties " +
                        "  expline:" + expline +
                        "  expTime:" + expTime +
                        ", target_expline:" + target_expline +
                        ", gain:" + gain +
                        ", gainRed:" + gainRed +
                        ", gainGreen:" + gainGreen +
                        ", gainBlue:" + gainBlue);

                if (aeRatio != 0) {
                    float ratio = 1 + (aeRatio/100);
                    int expline2 = (int)(ratio * expline);
                    int gain2 = (int)(ratio * gain);
                    LogService.i(TAG, "manual ctrl ratio " + ratio);
                    LogService.i(TAG, "manual ctrl set expline from " + expline + " to " + expline2);
                    LogService.i(TAG, "manual ctrl set gain from " + gain + " to " + gain2);
                    CameraConfigService.set("persist.camera.ov8856.shutter", String.valueOf(expline2));
                    CameraConfigService.set("persist.camera.ov8856.gain", String.valueOf(gain2));
                    CameraConfigService.set("persist.vendor.camera.bypass.ae", String.valueOf(1));
                    CameraConfigService.set("persist.vendor.camera.bypass.awb", String.valueOf(1));
                }

                if(awbRatio != 0) {
                    float ratio = 1 + (awbRatio/100);
                    int gainRed2 = (int)(ratio * gainRed);
                    int gainGreen2 = (int)(ratio * gainGreen);
                    int gainBlue2 = (int)(ratio * gainBlue);
                    LogService.i(TAG, "manual ctrl ratio " + ratio);
                    LogService.i(TAG, "manual ctrl set gainRed from " + gainRed + " to " + gainRed2);
                    LogService.i(TAG, "manual ctrl set gainGreen from " + gainGreen + " to " + gainGreen2);
                    LogService.i(TAG, "manual ctrl set gainBlue from " + gainBlue + " to " + gainBlue2);
                    CameraConfigService.set("debug.isp.awb.rgain.set", String.valueOf(gainRed2));
                    CameraConfigService.set("debug.isp.awb.ggain.set", String.valueOf(gainGreen2));
                    CameraConfigService.set("debug.isp.awb.bgain.set", String.valueOf(gainBlue2));
                    CameraConfigService.set("persist.vendor.camera.bypass.ae", String.valueOf(1));
                    CameraConfigService.set("persist.vendor.camera.bypass.awb", String.valueOf(1));
                }

                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    public void run() {
                        String exptime = CameraConfigService.get("debug.calibration.exp","3500");
                        _recordingService.setPreviewTexture(mCamMTexture);
                        ResSwitchchk.setClickable(false);
                        if(ResSwitchchk.isChecked()) {
                            CameraConfigService.setCamera17FPS(5,Long.parseLong(exptime));
                            _recordingService.setCaptureEvent(null,CAPTURE_BUFFER_8M);
                        } else {
                            CameraConfigService.setCamera17FPS(20,Long.parseLong(exptime));
                            _recordingService.setCaptureEvent(null,CAPTURE_BUFFER_2M);
                        }

                        _recordingService.start();
                    }
                });

            }
        });

        mCamCloseBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                _recordingService.stopStream();
                _recordingService.close();
                ResSwitchchk.setClickable(true);
                LocalConfigService.GetLastAEAWBValueJNI();
            }
        });

        mCaptureBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                _recordingService.setCaptureEvent(null,CAPTURE_SINGLE);
            }
        });

        mFloodBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                _recordingService.startFlood();
            }
        });

        mProjectorBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                _recordingService.startProjector();
            }
        });

        /* check and create folder */
        DiskService.createDirectory(new File(ARC_APP_PATH + "calibrationtool/L"));
        DiskService.createDirectory(new File(ARC_APP_PATH + "calibrationtool/R"));
        DiskService.createDirectory(new File(ARC_APP_PATH + "calibrationtool/M"));

        mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);

        mWifiOpenButton.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                wifiTestUtil = null;
                wifiTestUtil = new WifiTestUtil(mWifiManager) {

                    public void wifiStateChange(int newState) {
                        LogService.d(TAG, "wifiStateChange newState="+newState);
                        switch (newState) {
                            case WifiManager.WIFI_STATE_ENABLED:
                                mWifiState.setText("Wifi ON,Discovering...");
                                mIs5GHzBandSupported = mWifiManager.is5GHzBandSupported();
                                break;
                            case WifiManager.WIFI_STATE_DISABLED:
                                //tvWifiState.setText("Wifi OFF");
                                break;
                            case WifiManager.WIFI_STATE_DISABLING:
                                mWifiState.setText("Wifi Closing");
                                break;
                            case WifiManager.WIFI_STATE_ENABLING:
                                mWifiState.setText("Wifi Opening");
                                break;
                            case WifiManager.WIFI_STATE_UNKNOWN:
                            default:
                                mWifiState.setText("Wifi state Unknown");
                                // do nothing
                                break;

                        }
                    }

                    public void wifiDeviceListChange(List<ScanResult> wifiDeviceList) {
                        if (wifiDeviceList == null) {
                            return;
                        }

                        //tvWifiDeviceList.setText("");
                        LogService.d(TAG, "wifiDeviceListChange mIs5GHzBandSupported="+mIs5GHzBandSupported);
                        boolean isfound = false;
                        for (ScanResult result : wifiDeviceList) {
                            LogService.d(TAG, "device name: " + result.SSID + ", signal level: " + String.valueOf(result.level));
                            LogService.d(TAG, "result : " +result.toString());
                            HostName = CameraConfigService.get("debug.wifitest.hostname", "Bellus3d");
                            HostLevelThres = CameraConfigService.getInt("debug.wifitest.strenghlevel", -50);
                            if(HostName.equals(result.SSID) && (HostLevelThres > result.level || result.level != 0)) {
                                isfound = true;
                                break;
                            }
                        }
                        CameraConfigService.set("debug.wifitest.done", "1");
                        if(isfound) {
                            CameraConfigService.set("debug.wifitest.result", "1");
                            mWifiState.setText("Wifi Test Pass");
                        } else {
                            CameraConfigService.set("debug.wifitest.result", "0");
                            mWifiState.setText("Wifi Test Failed");
                        }
                        wifiTestUtil.stopTest();
                    }
                };
                CameraConfigService.set("debug.wifitest.done", "0");
                CameraConfigService.set("debug.wifitest.result", "0");
                wifiTestUtil.startTest(CalibrationTool.this);
            }
        });

//        mWifiCloseButton.setOnClickListener(new View.OnClickListener() {
//
//            @Override
//            public void onClick(View v) {
//                wifiTestUtil.stopTest();
//            }
//        });



    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_calibration_tool);

        mCaptureBtn     = (Button) findViewById(R.id.CaptureButton);
        mCamOpenBtn   = (Button) findViewById(R.id.CamOpenButton);
        mCamCloseBtn   = (Button) findViewById(R.id.CamCloseButton);
        mFloodBtn       = (Button) findViewById(R.id.FloodButton);
        mProjectorBtn    = (Button) findViewById(R.id.ProjectorButton);
        mCamOpenBtn.setClickable(false);
        mCamCloseBtn.setClickable(false);
        mCaptureBtn.setClickable(false);

        mWifiOpenButton = (Button) findViewById(R.id.WifiOpenButton);

        mWifiState = (TextView) findViewById(R.id.wifi_state_content);

        mFPS = (TextView) findViewById(R.id.fps_state_content);

        ResSwitchchk    = (CheckBox) findViewById(R.id.ResSwitchchk);

        ResSwitchchk.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                //is chkIos checked?
                if (((CheckBox) v).isChecked()) {
                    ResSwitchchk.setText("8M");
                } else
                    ResSwitchchk.setText("2M");
            }
        });

        _recordingService = new RecordingService();

        _permissionService = new PermissionService(this,permissions);
        if(_permissionService.checkAllPermissionsEnabled()) {
            setupUICompoment();
        } else {
            _permissionService.requestMultiplePermissions();
        }
    }

    @Override
    public void onStop() {
        super.onStop();
        _recordingService.stopFlood();
        _recordingService.stopProject();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
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
            _processingService.addFrameToNativeJNI(byteData,mTotalFramesStreamed,currentTimeStamp,mResolution,isCapture);
            //            if(depthCamera.getCaptureMilliseconds() != 0.0) {
//                final float fps = (float) depthCamera.getCapturedFrames() / (float) depthCamera.getCaptureMilliseconds() * 1000;
//                runOnUiThread(new Runnable() {
//                    @Override
//                    public void run() {
//                        NumberFormat formatter = new DecimalFormat("0.00");
//                        mFPS.setText(formatter.format(fps));
//                    }
//                });
//            }
        }
    };

    private TextureView.SurfaceTextureListener mSurfaceTextureListener = new TextureView.SurfaceTextureListener() {
        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            if (mCamLTexture.isAvailable() && mCamRTexture.isAvailable() && mCamMTexture.isAvailable()) {
                LogService.e(TAG, "onSurfaceTextureAvailable setPreviewTarget");
                //depthCamera.setPreviewTarget(mCamLTexture, mCamRTexture, mCamMTexture);
                _recordingService.setPreviewTarget(mCamLTexture, mCamRTexture, mCamMTexture);
                mCamOpenBtn.setClickable(true);
                mCamCloseBtn.setClickable(true);
                mCaptureBtn.setClickable(true);
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