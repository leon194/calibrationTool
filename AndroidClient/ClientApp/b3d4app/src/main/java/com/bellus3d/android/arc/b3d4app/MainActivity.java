package com.bellus3d.android.arc.b3d4app;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.TextureView;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;

import com.bellus3d.android.arc.b3d4client.AppStatusManager;
import com.bellus3d.android.arc.b3d4client.ConfigService.CameraConfigService;
import com.bellus3d.android.arc.b3d4client.ConfigService.LocalConfigService;
import com.bellus3d.android.arc.b3d4client.DiskService;
import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.ErrorReportingService.ErrorReportingService;
import com.bellus3d.android.arc.b3d4client.FormattingService;
import com.bellus3d.android.arc.b3d4client.GlobalResourceService;
import com.bellus3d.android.arc.b3d4client.JsonModel.*;
import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.*;
import com.bellus3d.android.arc.b3d4client.LogService.AndroidExceptionHandler;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.MMFrameBuffer;
import com.bellus3d.android.arc.b3d4client.NetworkService.B3DNetworkingListener;
import com.bellus3d.android.arc.b3d4client.NetworkService.NetworkService;
import com.bellus3d.android.arc.b3d4client.PermissionsService;
import com.bellus3d.android.arc.b3d4client.ProcessingService;
import com.bellus3d.android.arc.b3d4client.RecordingService;
import com.bellus3d.android.arc.b3d4client.StreamHelper;
import com.bellus3d.android.arc.b3d4client.camera.B3D4NativeProcessorListener;
import com.bellus3d.android.arc.b3d4client.camera.B3DCaptureSettings;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraError;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraObserver;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraState;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraStreamListener;
import com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage;
import com.google.gson.Gson;

import org.apache.commons.io.FileUtils;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_SENSOR_STREAMING_ERROR.*;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.*;
import static com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage.HostCommands.*;
import static com.bellus3d.android.arc.b3d4client.AppStatusManager.AppState.*;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.CaptureEvent.*;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.TripleCamResolution.*;

public class MainActivity extends AppCompatActivity {


    private static String _deviceID = GlobalResourceService.getDeviceId();

    B3DCaptureSettings _mCaptureSettings;
    private DepthCameraState.StateType _mDepthCameraState = DepthCameraState.StateType.CONNECTED;
    private NetWorkMessage.HostCommands _prevHostCommands = CMD_END;
    private NetWorkMessage.HostCommands _curHostCommands = CMD_END;
    private Lock cmdLock = new ReentrantLock();

    Button mCaptureBtn, mCamOpenBtn, mCamCloseBtn, mFloodBtn, mProjectorBtn, mSDUpdate;

    Button mStartCaptureButton, mMergeButton;

    Button mStopMonitorButton;

    CheckBox ResSwitchchk;

    private TextureView mCamLTexture, mCamRTexture, mCamMTexture;

    RecordingService _recordingService;
    PermissionsService _permissionsService;
    ProcessingService _processingService;
    NetworkService _networkService;
    LocalConfigService _configService;
    ErrorReportingService _errorReportingService;
    GlobalResourceService _globalResourceService;
    DeviceConfig _deviceConfig;

    private StreamHelper _streamStatistics = new StreamHelper();

    HandlerThread mClientProtectThread;
    Handler mClientProtectHandler;
    boolean mClientProtectControl;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        AppStatusManager.getInstance().setState(CAMERA_STARED);

        _globalResourceService = new GlobalResourceService(this);

        if(!(Thread.getDefaultUncaughtExceptionHandler() instanceof AndroidExceptionHandler)) {
            Thread.setDefaultUncaughtExceptionHandler(new AndroidExceptionHandler());
        }

        CURRENT_LOG_LEVEL = Integer.parseInt(CameraConfigService.get("debug.bellus3d.client.loglevel","2")); //level 2 is INFO, default set to info
        LogService.setLogLevel(LogService.Level.fromInteger(CURRENT_LOG_LEVEL));

        _permissionsService = new PermissionsService(this);

        if (!_permissionsService.checkAllPermissionsEnabled()){
            _permissionsService.checkAndRequestPermissions(new PermissionsService.PermissionsGrantedCallback() {
                @Override
                public void onPermissionsGranted() {
                    afterPermissionGranted();
                    setupUICompoment();
                }
            });
        }

        mCaptureBtn = findViewById(R.id.CaptureButton);
        mCamOpenBtn = findViewById(R.id.CamOpenButton);
        mCamCloseBtn = findViewById(R.id.CamCloseButton);
        mFloodBtn = findViewById(R.id.FloodButton);
        mProjectorBtn = findViewById(R.id.ProjectorButton);
        mCamOpenBtn.setClickable(false);
        mCamCloseBtn.setClickable(false);
        mCaptureBtn.setClickable(false);
        mStartCaptureButton = findViewById(R.id.StartCaptureButton);
        mMergeButton = findViewById(R.id.MergeButton);
        mStopMonitorButton = findViewById(R.id.stopMonitorButton);
        mSDUpdate  = findViewById(R.id.sdUpdateButton);

        ResSwitchchk = findViewById(R.id.ResSwitchchk);

        _recordingService = new RecordingService();
        _permissionsService = new PermissionsService(this);
        _processingService = new ProcessingService();
        _errorReportingService = new ErrorReportingService();
        _networkService = new NetworkService(this);

        if (_permissionsService.checkAllPermissionsEnabled()){
            WRITE_TO_FILE = true;
            afterPermissionGranted();
            setupUICompoment();
        }
    }

    private void resetCurCommand() {
        cmdLock.lock();
        _curHostCommands = CMD_END;
        cmdLock.unlock();
    }
    private void afterPermissionGranted(){
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyyMMdd_HHmmss");
        final String timestamp = simpleDateFormat.format(new Date());
        final String logFoldername = CameraConfigService.get("debug.bellus3d.logfoldername",timestamp);

        LogService.initLogFile(logFoldername);
        WRITE_TO_FILE = true;
        DiskService.createDirectory(new File(LOGS_FOLDER_PATH + "/" + logFoldername));
        _configService = new LocalConfigService();

        DiskService.createDirectory(new File(CONFIGS_FOLDER_PATH));
        DiskService.createDirectory(new File(SESSIONS_FOLDER_PATH));
        DiskService.createDirectory(new File(LOGS_FOLDER_PATH));

        _deviceConfig = _configService.loadConfigFile(CONFIG_FILE_PATH);
        if (_deviceConfig == null) {
            LogService.e(TAG, "Error loading file : " + CONFIG_FILE_PATH);
            _deviceConfig = new DeviceConfig();
            _deviceConfig.setLayoutPosition("c");
            _deviceConfig.setLayoutDevices(1);
            _deviceConfig.setLayoutName("single");
            _deviceConfig.setLocalConnection(true);
            _deviceConfig.setHostAddress("127.0.0.1");
            _deviceConfig.setUdpPort(3000);
            _deviceConfig.setHttpPort(3001);
            _deviceConfig.setWsPort(3002);
            _deviceConfig.setHotspot("B3D4_any");
            _deviceConfig.setPassword("HappyB3D");
            _configService.saveConfigFile(_deviceConfig);
        }

        _processingService.setDeviceConfig(_deviceConfig);
        _processingService.registerNativeProcessorListener(b3D4NativeProcessorListener);
        _networkService.setDeviceConfig(_deviceConfig);
        _networkService.registerNetWorkingListener(mNetWorkingListener);
        _networkService.registerWifiReceiver();
        _recordingService.setNetworkService(_networkService);
        _recordingService.setDeviceConfig(_deviceConfig);
        _processingService.setNetworkService(_networkService);
        _errorReportingService.setNetworkService(_networkService);
        _networkService.initializeNetwork();
    }

    private void setupUICompoment() {
        /* create Monitor Service after permission generated */
        Intent intent = new Intent(MainActivity.this, MonitoringService.class);
        intent.setAction("android.intent.action.RESPOND_VIA_MESSAGE");
        startService(intent);

        mCamLTexture = (TextureView) findViewById(R.id.camLTexture);
        mCamRTexture = (TextureView) findViewById(R.id.camRTexture);
        mCamMTexture = (TextureView) findViewById(R.id.camMTexture);

        mCamLTexture.setSurfaceTextureListener(mSurfaceTextureListener);
        mCamRTexture.setSurfaceTextureListener(mSurfaceTextureListener);
        mCamMTexture.setSurfaceTextureListener(mSurfaceTextureListener);

        _recordingService.initializeCamera(this);
        _recordingService.setPreviewTexture(mCamMTexture);
        _recordingService.registerStreamListener(mRecordingServiceListener);
        _recordingService.registerObserver(mRecordingServiceObserver);

        /* onSurfaceTextureAvailable does not get called if it is already available */
        if (mCamLTexture.isAvailable() && mCamRTexture.isAvailable() && mCamMTexture.isAvailable()) {
            _recordingService.setPreviewTexture(mCamMTexture);
            mCamOpenBtn.setClickable(true);
            mCamCloseBtn.setClickable(true);
            mCaptureBtn.setClickable(true);
        }

        mCamOpenBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    public void run() {
                        ResSwitchchk.setClickable(false);
                        if(ResSwitchchk.isChecked()) {
                            CameraConfigService.setCamera17FPS(5,(long)(3.5*1000));
                            _recordingService.setCaptureEvent(null,CAPTURE_BUFFER_8M);
                        } else {
                            CameraConfigService.setCamera17FPS(20,(long)(3.5*1000));
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
            }
        });

        mCaptureBtn.setOnClickListener(new View.OnClickListener() {
            @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
            @Override
            public void onClick(View v) {
                /* receive buffer_capture event */
                /* each command gets a new capture settings */
                _mCaptureSettings = _recordingService.getCaptureSettings();

                /* get network message */

                /* enable native start add frame flag before start stream */
                _processingService.InitProcessingParemeter(ProcessingService.PROCESS_TYPE.PROCESS_TYPE_SINGLEVIEW.getValue());
                _recordingService.runBufferCapture(_mCaptureSettings, null);
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

        mMergeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //TODO
            }
        });

        mStopMonitorButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setAction("kill_self");
                sendOrderedBroadcast(intent, null);
                //am broadcast -a "kill_self"
            }
        });

        ResSwitchchk.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                if (((CheckBox) v).isChecked()) {
                    ResSwitchchk.setText("8M");
                } else
                    ResSwitchchk.setText("2M");
            }
        });

        mSDUpdate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent("com.revo.action.REVO_LOCAL_SYSTEM_UPDATE");
                intent.setPackage("com.android.settings");
                sendBroadcast(intent);
            }
        });

        GlobalResourceService.checkCalibrationFiles();

        GlobalResourceService.checkSTasmFiles();

        /* set working root directory to native */
        LocalConfigService.setArcNativeRootPathJNI(ARC_CLIENT_PATH);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    // This seems to be required in order for onDestroy to be called.
    @Override
    public boolean isFinishing() {
        return super.isFinishing();
    }

    @Override
    protected void onDestroy() {
        _networkService.unRegisterWifiReceiver();
        super.onDestroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults){
        if (_permissionsService.checkAllPermissionsEnabled()){
            setupUICompoment();
        }
    }

    /* a listener that returns frame from java camera API */
    private final DepthCameraStreamListener mRecordingServiceListener = new DepthCameraStreamListener() {

        @Override
        public void onWarning(B3D4ClientError error, B3DCaptureSettings captureSetting) {
            _errorReportingService.sendWarningToHost(error,captureSetting);
        }

        @Override
        public void onError(B3D4ClientError error, B3DCaptureSettings captureSetting) {
            _errorReportingService.sendErrorToHost(error,captureSetting);
            String request = captureSetting.getRequest().toLowerCase();
            LogService.d(TAG,"current request : " + request);
            if( request.equals(STRING_BUFFER_CAPTURE) ||
                    request.equals(STRING_BUFFER_MERGE)||
                    request.equals(STRING_CANCEL_PROCESS) ) {
                // reset curHostCommand when error occur
                resetCurCommand();
            }
        }

        @Override
        public void onStatusHost(B3DCaptureSettings mCaptureSetting, String status) {
            Gson gson = new Gson();
            String request = mCaptureSetting.getRequest().toLowerCase();
            switch(request) {
                case STRING_STREAM_START : {
                    StreamStartResponseTo streamStartResponseTo = new StreamStartResponseTo();
                    streamStartResponseTo.setStatus(status);
                    streamStartResponseTo.setResponseTo(mCaptureSetting.getRequest());
                    streamStartResponseTo.setRequestId(mCaptureSetting.getRequestID());
                    streamStartResponseTo.setCamera(_deviceConfig.getLayoutPosition());
                    streamStartResponseTo.setDeviceId(_deviceID);
                    streamStartResponseTo.setStart(String.valueOf(_streamStatistics.tsStreamStart));
                    String streamStartMsg = gson.toJson(streamStartResponseTo);
                    _networkService.sendMessage(streamStartMsg);
                    break;
                }
                case STRING_BUFFER_CAPTURE : {
                    // after buffer_capture save all the cached buffer to memory/mmap
                    // still need to check whether the camera is closed or not
                    // this should only take less than a sec, so currently not to new a thread to
                    // do this
                    stopClientProtect(); // buffer_capture
                    long startTime = System.currentTimeMillis();
                    while (!((_mDepthCameraState == DepthCameraState.StateType.CONNECTED) ||
                            (System.currentTimeMillis() - startTime) > 5000)) {
                        LogService.i(TAG, "mDepthCameraState : " + _mDepthCameraState.getName());
                        try {
                            Thread.sleep(30);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                            LogService.logStackTrace(TAG, e.getStackTrace());
                        }
                    }
                    BufferIdResponseTo bufferIdResponseTo = new BufferIdResponseTo();
                    bufferIdResponseTo.setStatus(status);
                    bufferIdResponseTo.setResponseTo(mCaptureSetting.getRequest());
                    bufferIdResponseTo.setRequestId(mCaptureSetting.getRequestID());
                    bufferIdResponseTo.setBufferId(mCaptureSetting.getCaptureBufferID());
                    String bufferIdResponseToMsg = gson.toJson(bufferIdResponseTo);
                    LogService.i(TAG,"send response to host " + bufferIdResponseToMsg);
                    _networkService.sendMessage(bufferIdResponseToMsg);
                    break;
                }
                default: {
                    stopClientProtect(); // stream_depth
                    ResponseTo responseTo = new ResponseTo();
                    responseTo.setRequestId(mCaptureSetting.getRequestID());
                    responseTo.setResponseTo(mCaptureSetting.getRequest());
                    responseTo.setStatus(status);
                    String responseToStr = gson.toJson(responseTo);
                    _networkService.sendMessage(responseToStr);
                    LogService.i(TAG,"send response to host " + responseToStr);
                    AppStatusManager.getInstance().setState(AppStatusManager.AppState.WEBSOCKET_CONNECTED);
                }
            }
        }

        public void onStatusHostSub(B3DCaptureSettings mCaptureSetting, String status) {
            Gson gson = new Gson();
            String request = mCaptureSetting.getRequest().toLowerCase();
            String subaction = mCaptureSetting.getSubRequest().toLowerCase();

            LogService.v(TAG,"request : " + request + ", subaction : " + subaction);

            switch(request) {
                case STRING_STREAM_START : {
                    if (subaction.equals(STRING_BUFFER_SNAPSHOT)) {
                        ResponseTo bufferSnapResponseTo = new ResponseTo();
                        bufferSnapResponseTo.setStatus(NetWorkMessage.Status.OK.getName());
                        bufferSnapResponseTo.setResponseTo(mCaptureSetting.getSubRequest());
                        bufferSnapResponseTo.setRequestId(mCaptureSetting.getSubRequestID());
                        String bufferSnapMsg = gson.toJson(bufferSnapResponseTo);
                        _networkService.sendMessage(bufferSnapMsg);
                        LogService.i(TAG,"send sub response to host " + bufferSnapMsg);
                    }
                    break;
                }
                default: {
                    ResponseTo responseTo = new ResponseTo();
                    responseTo.setRequestId(mCaptureSetting.getSubRequestID());
                    responseTo.setResponseTo(mCaptureSetting.getSubRequest());
                    responseTo.setStatus(status);
                    String responseToStr = gson.toJson(responseTo);
                    _networkService.sendMessage(responseToStr);
                    LogService.i(TAG,"send sub response to host " + responseToStr);
                }
            }
        }

        @Override
        public void onMsgHost(B3DCaptureSettings mCaptureSetting, String msg) {
            LogService.i(TAG, msg);
            _networkService.sendMessage(msg);
        }

        @Override
        public void onNotificationHost(B3DCaptureSettings mCaptureSetting, long timeStampL, long timeStampR, long frameIndex) {
            Gson gson = new Gson();
            String request = mCaptureSetting.getRequest();
            LogService.d(TAG,"L/R diff " + (timeStampL-timeStampR));
            switch (request) {
                case STRING_STREAM_DEPTH :
                case STRING_STREAM_CAPTURE:
                case STRING_BUFFER_CAPTURE: {
                    if(frameIndex == 0 && (Math.abs(timeStampL-timeStampR) >= 100) && request.toLowerCase().equals(STRING_STREAM_DEPTH))
                        _processingService.updateNativeDebugFlag(mCaptureSetting.getStreamDepthBufferID(),true);

                    TimeStampNotification timeStampNotification = new TimeStampNotification();
                    timeStampNotification.setNotification("ir_timestamp");
                    timeStampNotification.setCaptureType(mCaptureSetting.getRequest());
                    timeStampNotification.setFrameIndex(String.valueOf(frameIndex));
                    timeStampNotification.setTimeStampL(String.valueOf(timeStampL));
                    timeStampNotification.setTimeStampR(String.valueOf(timeStampR));
                    String timeStampNotificationStr = gson.toJson(timeStampNotification);
                    _networkService.sendMessage(timeStampNotificationStr);
                    LogService.i(TAG,"send notification to host " + timeStampNotificationStr);
                    break;
                }
            }
        }

        public void onFrameHostSub(final B3DCaptureSettings captureSettings, final byte[] byteData, long frameID) {
            final Gson gson = new Gson();
            String subaction = captureSettings.getSubRequest().toLowerCase();

            switch(captureSettings.getRequest().toLowerCase()) {
                case STRING_STREAM_START: {
                    if (subaction.equals(STRING_BUFFER_SNAPSHOT_RECEIVE)) {
                        final long frame_index =_streamStatistics.bufferSnapShotIndex++;

                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                long timeStampL = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                        (Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_IRL_START, ARC_2M_TIMESTAMP_IRR_START)))).getLong();
                                long timeStampR = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                        (Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_IRR_START, ARC_2M_TIMESTAMP_IRR_END)))).getLong();
                                if (frame_index == 0) {
                                    long timeStampM = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                            (Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_COLOR_START, ARC_2M_TIMESTAMP_IRL_START)))).getLong();
                                    byte[] colorbyte = Arrays.copyOfRange(byteData, 0, ARC_2M_COLOR_SIZE);
                                    String fileNameM = String.format("%03d", frame_index) +
                                            "_" + String.format("%06d", timeStampM / NSTOMS) + "_" + String.format("%05d", frame_index);
                                    SnapShotReceiveRequest bufferSnapshotReceiveRequest = new SnapShotReceiveRequest();
                                    bufferSnapshotReceiveRequest.setRequestId(captureSettings.getSubRequestID());
                                    bufferSnapshotReceiveRequest.setRequest(captureSettings.getSubRequest());
                                    bufferSnapshotReceiveRequest.setRequestType(REQUEST_TYPE_SNAPSHOT);
                                    bufferSnapshotReceiveRequest.setFrameType(FRAME_TYPE_COLOR);
                                    bufferSnapshotReceiveRequest.setPosition(captureSettings.getBufferSnapCamPose());
                                    bufferSnapshotReceiveRequest.setFileNameM(fileNameM);
                                    bufferSnapshotReceiveRequest.setFileSize(String.valueOf(colorbyte.length));
                                    bufferSnapshotReceiveRequest.setBufferId(captureSettings.getBufferSnapReceiveBufferID());
                                    bufferSnapshotReceiveRequest.setBufferFinish(STRING_TRUE);

                                    String bufferSnapshotReceiveRequestStr = gson.toJson(bufferSnapshotReceiveRequest);
                                    _networkService.sendBinaryToHost(bufferSnapshotReceiveRequestStr, colorbyte, true);
                                    LogService.d(TAG, "send bufferSnapshot color image to host " + bufferSnapshotReceiveRequestStr);
                                }

                                if (frame_index < captureSettings.getBufferSnapFrames()) {
                                    boolean isDone = (frame_index == captureSettings.getBufferSnapFrames() - 1);
                                    String fileNameL = String.format("%03d", frame_index) +
                                            "_" + String.format("%06d", timeStampL / NSTOMS) + "_" + String.format("%05d", frame_index);
                                    String fileNameR = String.format("%03d", frame_index) +
                                            "_" + String.format("%06d", timeStampR / NSTOMS) + "_" + String.format("%05d", frame_index);

                                    byte[] stereobyte = Arrays.copyOfRange(byteData, ARC_2M_COLOR_SIZE, ARC_2M_COLOR_SIZE + ARC_2M_IR_SIZE * 2);

                                    SnapShotReceiveRequest bufferSnapshotReceiveRequest = new SnapShotReceiveRequest();
                                    bufferSnapshotReceiveRequest.setRequestId(captureSettings.getSubRequestID());
                                    bufferSnapshotReceiveRequest.setRequest(captureSettings.getSubRequest());
                                    bufferSnapshotReceiveRequest.setRequestType(REQUEST_TYPE_SNAPSHOT);
                                    bufferSnapshotReceiveRequest.setFrameType(FRAME_TYPE_STEREO);
                                    bufferSnapshotReceiveRequest.setPosition(captureSettings.getBufferSnapCamPose());
                                    bufferSnapshotReceiveRequest.setFileNameL(fileNameL);
                                    bufferSnapshotReceiveRequest.setFileNameR(fileNameR);
                                    bufferSnapshotReceiveRequest.setFileSize(String.valueOf(stereobyte.length));
                                    bufferSnapshotReceiveRequest.setBufferId(captureSettings.getBufferSnapReceiveBufferID());
                                    bufferSnapshotReceiveRequest.setBufferFinish((isDone ? STRING_TRUE : STRING_FALSE));

                                    String bufferSnapshotReceiveRequestStr = gson.toJson(bufferSnapshotReceiveRequest);
                                    _networkService.sendBinaryToHost(bufferSnapshotReceiveRequestStr, stereobyte, true);
                                    LogService.d(TAG, "send bufferSnapshot stereo image to host " + bufferSnapshotReceiveRequestStr);

                                    if(isDone) {
                                        ResponseTo bufferSnapReceiveResponseTo = new ResponseTo();
                                        bufferSnapReceiveResponseTo.setStatus(NetWorkMessage.Status.OK.getName());
                                        bufferSnapReceiveResponseTo.setResponseTo(captureSettings.getSubRequest());
                                        bufferSnapReceiveResponseTo.setRequestId(captureSettings.getSubRequestID());
                                        String bufferSnapReceiveResponseMsg = gson.toJson(bufferSnapReceiveResponseTo);
                                        _networkService.sendMessage(bufferSnapReceiveResponseMsg);
                                        LogService.i(TAG, "send ok response to host " + bufferSnapReceiveResponseMsg);
                                    }
                                }
                            }
                        }).start();
                    }
                    break;
                }
            }
        }
        @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
        @Override
        public void onFrameHost(B3DCaptureSettings captureSettings, byte[] byteData, long frameID) {
            LogService.v(TAG, "");
            Gson gson = new Gson();

            switch(captureSettings.getRequest().toLowerCase()) {
                case STRING_STREAM_START :{
                    _streamStatistics.framesCaptured++;
                    if (_streamStatistics.framesStreamed == 0) {
                        StreamStartResponseTo streamStartResponseTo = new StreamStartResponseTo();
                        streamStartResponseTo.setStatus(NetWorkMessage.Status.OK.getName());
                        streamStartResponseTo.setResponseTo(captureSettings.getRequest());
                        streamStartResponseTo.setRequestId(captureSettings.getRequestID());
                        streamStartResponseTo.setCamera(_deviceConfig.getLayoutPosition());
                        streamStartResponseTo.setDeviceId(_deviceID);
                        streamStartResponseTo.setStart(String.valueOf(_streamStatistics.tsStreamStart));
                        String streamStartMsg = gson.toJson(streamStartResponseTo);
                        _networkService.sendMessage(streamStartMsg);
                    }

                    if (_streamStatistics.requestedFPS != 0.0) {
                        double duration = (System.currentTimeMillis() - _streamStatistics.tsStreamStart) / 1000.0;
                        if (duration != 0.0) {
                            double fps = _streamStatistics.framesStreamed / duration;
                            if (fps > _streamStatistics.requestedFPS) {
                                return;
                            }
                        }
                    }
                    _streamStatistics.streamedBytes += byteData.length;
                    _streamStatistics.framesStreamed++;

                    if (captureSettings.getStreamFrames() > 0 && _streamStatistics.framesStreamed >= captureSettings.getStreamFrames()) {
                        if (_streamStatistics.framesStreamed > captureSettings.getStreamFrames())
                            return;
                        _recordingService.runStopStream(_streamStatistics, _mCaptureSettings);
                        return;
                    }

                    _streamStatistics.streamWidth = captureSettings.getWidth();
                    _streamStatistics.streamHeight = captureSettings.getHeight();

                    StreamRequest streamRequest = new StreamRequest();
                    streamRequest.setRequest(NetWorkMessage.RequestType.STREAM.getName());
                    streamRequest.setRequestId(captureSettings.getRequestID());
                    streamRequest.setDeviceId(_deviceID);
                    streamRequest.setCamera(_deviceConfig.getLayoutPosition());
                    streamRequest.setFrameIndex(String.valueOf(_streamStatistics.framesStreamed));
                    streamRequest.setWidth(String.valueOf(_streamStatistics.streamWidth));
                    streamRequest.setHeight(String.valueOf(_streamStatistics.streamHeight));
                    streamRequest.setFileSize(String.valueOf(byteData.length));
                    streamRequest.setTs(String.valueOf(_streamStatistics.tsStreamStart));

                    Tracking tracking = new Tracking();
                    FaceTrackingInfo faceTrackingInfo = new FaceTrackingInfo();
                    Position distance = new Position();
                    Position rotation = new Position();
                    Position faceRect = new Position();

                    distance.setX(String.valueOf(captureSettings.getTrackingFaceX()));
                    distance.setY(String.valueOf(captureSettings.getTrackingFaceY()));
                    distance.setZ(String.valueOf(captureSettings.getTrackingFaceZ()));

                    rotation.setX(String.valueOf(captureSettings.getTrackingFaceRotationX()));
                    rotation.setY(String.valueOf(captureSettings.getTrackingFaceRotationY()));
                    rotation.setZ(String.valueOf(captureSettings.getTrackingFaceRotationZ()));

                    faceRect.setX(String.valueOf(captureSettings.getTrackingFaceFaceRectX()));
                    faceRect.setY(String.valueOf(captureSettings.getTrackingFaceFaceRectY()));
                    faceRect.setWidth(String.valueOf(captureSettings.getTrackingFaceFaceRectWidth()));
                    faceRect.setHeight(String.valueOf(captureSettings.getTrackingFaceFaceRectHeight()));

                    faceTrackingInfo.setDistance(distance);
                    faceTrackingInfo.setRotation(rotation);
                    faceTrackingInfo.setFaceRect(faceRect);

                    tracking.setFaceTrackingInfo(faceTrackingInfo);
                    streamRequest.setTracking(tracking);

                    String streamRequestMsg = gson.toJson(streamRequest);
                    _networkService.sendBinaryToHost(streamRequestMsg, byteData, true);
                    LogService.d(TAG, streamRequestMsg);
                    break;
                }
                case STRING_BUFFER_CAPTURE :{
                    byte[] colorFrame = new byte[0];
                    if(captureSettings.getBufferCaptureMode().equals(BUFFER_CAPTURE_STILL))
                        colorFrame = FormattingService.compressYuvImg(Arrays.copyOfRange(byteData, 0, ARC_8M_COLOR_HEIGHT * ARC_8M_COLOR_WIDTH * 3 / 2),
                                ImageFormat.NV21, ARC_8M_COLOR_WIDTH, ARC_8M_COLOR_HEIGHT, null, "JPEG");
                    else if(captureSettings.getBufferCaptureMode().equals(BUFFER_CAPTURE_4D))
                        colorFrame = FormattingService.compressYuvImg(Arrays.copyOfRange(byteData, 0, ARC_2M_COLOR_HEIGHT * ARC_2M_COLOR_WIDTH * 3 / 2),
                                ImageFormat.NV21, ARC_2M_COLOR_WIDTH, ARC_2M_COLOR_HEIGHT, null, "JPEG");

                    Bitmap image = BitmapFactory.decodeByteArray(colorFrame, 0, colorFrame.length);
                    Bitmap resized = Bitmap.createScaledBitmap(image, 810, 1080, true);
                    ByteArrayOutputStream stream = new ByteArrayOutputStream();
                    resized.compress(Bitmap.CompressFormat.JPEG, 95, stream);

                    if(_mCaptureSettings.isReturnFirstFrame() || _mCaptureSettings.getBufferFrames() == 0) {
                        // buffer_capture for return first frame
                        String bufferSize;
                        byte[] bufferToSend = null;
                        if(_mCaptureSettings.getBufferFrames() == 0 ) {
                            /* use for buffer_capture with 0 , the frame comes from
                            *  DepthCamera_B3D4_SINGLE */
                            bufferToSend = byteData;
                            bufferSize = String.valueOf(bufferToSend.length);
                        } else {
                            /* use for return fist buffer_capture frame */
                            bufferToSend = stream.toByteArray();
                            bufferSize = String.valueOf(bufferToSend.length);
                        }

                        if(_mCaptureSettings.isDebug()) {
                            DiskService.createDirectory(new File(DEBUG_FOLDER_PATH));
                            DiskService.createDirectory(new File(BUFFER_CAPTURE_DEBUG_FOLDER_PATH));
                            String path = BUFFER_CAPTURE_DEBUG_FOLDER_PATH + "/" + _mCaptureSettings.getCaptureBufferID() +
                                    "_first_frame.jpg";
                            try (FileOutputStream fos = new FileOutputStream(path)) {
                                fos.write(stream.toByteArray());
                            } catch (FileNotFoundException e) {
                                e.printStackTrace();
                                LogService.logStackTrace(TAG, e.getStackTrace());
                            } catch (IOException e) {
                                e.printStackTrace();
                                LogService.logStackTrace(TAG, e.getStackTrace());
                            }
                        }

                        SendCaptureDataRequest sendCaptureDataRequest = new SendCaptureDataRequest();
                        sendCaptureDataRequest.setStatus(NetWorkMessage.Status.OK.getName());
                        sendCaptureDataRequest.setRequest(captureSettings.getRequest());
                        sendCaptureDataRequest.setRequestId(captureSettings.getRequestID());
                        sendCaptureDataRequest.setDeviceId(_deviceID);
                        sendCaptureDataRequest.setCamera(_deviceConfig.getLayoutPosition());
                        sendCaptureDataRequest.setFileSize(bufferSize);
                        sendCaptureDataRequest.setBufferId(captureSettings.getCaptureBufferID());

                        String captureRequestMsg = gson.toJson(sendCaptureDataRequest);
                        _networkService.sendBinaryToHost(captureRequestMsg, bufferToSend, true);
                        LogService.i(TAG, captureRequestMsg);
                    }

                    // buffer_capture with 0 will use DepthCamera_B3D4_SINGLE, and no need to send this
                    // other buffer_capture bufferFrames should grater than 0
                    if(_mCaptureSettings.getBufferFrames() > 0) {
                        int index = 0;
                        // send calibration data
                        File[] zipfiles = new File[4];
                        String[] paths = new String[4];
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/leftCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/midCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/rightCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/b3dCalibData.bin");

                        byte[] calibYmls = null;
                        calibYmls = DiskService.zipFiles(paths, zipfiles);

                        StreamCaptureRequest sendValidatorDataCalibRequest = new StreamCaptureRequest();
                        sendValidatorDataCalibRequest.setRequest(captureSettings.getRequest());
                        sendValidatorDataCalibRequest.setRequestId(captureSettings.getRequestID());
                        sendValidatorDataCalibRequest.setBufferId(captureSettings.getCaptureBufferID());
                        sendValidatorDataCalibRequest.setFilesize(String.valueOf(calibYmls.length));
                        sendValidatorDataCalibRequest.setFrameType(FRAME_TYPE_CALIZIP);

                        String sendValidatorDataCalibRequestStr = gson.toJson(sendValidatorDataCalibRequest);
                        _networkService.sendBinaryToHost(sendValidatorDataCalibRequestStr, calibYmls, true);
                        LogService.d(TAG, "send calibration zip to host " + sendValidatorDataCalibRequestStr);

                        //buffer_capture send first frame for validation
                        if(_mCaptureSettings.isDebug()) {
                            DiskService.createDirectory(new File(DEBUG_FOLDER_PATH));
                            DiskService.createDirectory(new File(BUFFER_CAPTURE_DEBUG_FOLDER_PATH));
                            String path = BUFFER_CAPTURE_DEBUG_FOLDER_PATH + "/" + _mCaptureSettings.getCaptureBufferID() +
                                    "_validation_frame.jpg";
                            try (FileOutputStream fos = new FileOutputStream(path)) {
                                fos.write(stream.toByteArray());
                            } catch (FileNotFoundException e) {
                                e.printStackTrace();
                                LogService.logStackTrace(TAG, e.getStackTrace());
                            } catch (IOException e) {
                                e.printStackTrace();
                                LogService.logStackTrace(TAG, e.getStackTrace());
                            }
                        }

                        long colorTimeStamp = 0L;
                        if(captureSettings.getBufferCaptureMode().equals(BUFFER_CAPTURE_STILL))
                            colorTimeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                                    Arrays.copyOfRange(byteData, ARC_8M_TIMESTAMP_COLOR_START, ARC_8M_TIMESTAMP_IRL_START)))).getLong() / 1000000;
                        else if(captureSettings.getBufferCaptureMode().equals(BUFFER_CAPTURE_4D))
                            colorTimeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                                    Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_COLOR_START, ARC_2M_TIMESTAMP_IRL_START)))).getLong() / 1000000;

                        String fileNameColor = String.format("%03d", 1) +
                                "_" + String.format("%06d", colorTimeStamp) + "_" + String.format("%05d", 1);

                        StreamCaptureRequest sendValidatorDataRequest = new StreamCaptureRequest();
                        sendValidatorDataRequest.setRequest(captureSettings.getRequest());
                        sendValidatorDataRequest.setRequestId(captureSettings.getRequestID());
                        sendValidatorDataRequest.setBufferId(captureSettings.getCaptureBufferID());
                        sendValidatorDataRequest.setFrameType(FRAME_TYPE_COLOR);
                        sendValidatorDataRequest.setFilesize(String.valueOf(stream.toByteArray().length));
                        sendValidatorDataRequest.setFilenameColor(fileNameColor);

                        String sendValidatorDataRequestStr = gson.toJson(sendValidatorDataRequest);
                        _networkService.sendBinaryToHost(sendValidatorDataRequestStr, stream.toByteArray(), true);
                        LogService.d(TAG, "send color image to host " + sendValidatorDataRequestStr);

                        long timeStampL = 0L;
                        byte[] stereoFrame = new byte[0];
                        byte[] depthFrame = new byte[0];

                        if(captureSettings.getBufferCaptureMode().equals(BUFFER_CAPTURE_STILL)) {
                            stereoFrame = Arrays.copyOfRange(byteData, ARC_8M_COLOR_SIZE, ARC_8M_TIMESTAMP_COLOR_START).clone();
                            depthFrame = _processingService.StartSingleDepthComputation(stereoFrame, ARC_8M_IR_WIDTH, ARC_8M_IR_HEIGHT, 0.75f);
                            timeStampL = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                    (Arrays.copyOfRange(byteData, ARC_8M_TIMESTAMP_IRL_START, ARC_8M_TIMESTAMP_IRR_START)))).getLong();
                        } else if(captureSettings.getBufferCaptureMode().equals(BUFFER_CAPTURE_4D)) {
                            stereoFrame = Arrays.copyOfRange(byteData, ARC_2M_COLOR_SIZE, ARC_2M_TIMESTAMP_COLOR_START).clone();
                            depthFrame = _processingService.StartSingleDepthComputation(stereoFrame, ARC_2M_IR_WIDTH, ARC_2M_IR_HEIGHT, 0.75f);
                            timeStampL = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                    (Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_IRL_START, ARC_2M_TIMESTAMP_IRR_START)))).getLong();
                        }

                        String fileNameL = String.format("%03d", 1) +
                                "_" + String.format("%06d", timeStampL / NSTOMS) + "_" + String.format("%05d", 1);

                        sendValidatorDataRequest.setRequest(captureSettings.getRequest());
                        sendValidatorDataRequest.setRequestId(captureSettings.getRequestID());
                        sendValidatorDataRequest.setBufferId(captureSettings.getCaptureBufferID());
                        sendValidatorDataRequest.setFrameType(FRAME_TYPE_DEPTH);
                        sendValidatorDataRequest.setFilesize(String.valueOf(depthFrame.length));
                        sendValidatorDataRequest.setFilenameColor(fileNameL);

                        sendValidatorDataRequestStr = gson.toJson(sendValidatorDataRequest);
                        _networkService.sendBinaryToHost(sendValidatorDataRequestStr, depthFrame, true);
                        LogService.d(TAG, "send depth image to host " + sendValidatorDataRequestStr);
                    }

                    if(captureSettings.getBufferFrames() == 0 &&
                            captureSettings.getBufferSource() == B3DCaptureSettings.CameraSource.COLOR) {
                        AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTED);
                        _recordingService.stopStream();
                        _recordingService.close();
                    }
                    break;
                }
                case STRING_STREAM_CAPTURE: {
                    _streamStatistics.bufferDepthRawIndex++;
                    /* send calibration files to host */
                    if (_streamStatistics.bufferDepthRawIndex == 1) {
                        int index = 0;
                        File[] zipfiles = new File[4];
                        String[] paths = new String[4];
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/leftCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/midCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/rightCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/b3dCalibData.bin");

                        byte[] calibYmls = null;
                        calibYmls = DiskService.zipFiles(paths, zipfiles);

                        StreamCaptureRequest streamCaptureCalibRequest = new StreamCaptureRequest();
                        streamCaptureCalibRequest.setRequest(captureSettings.getRequest());
                        streamCaptureCalibRequest.setRequestId(captureSettings.getRequestID());
                        streamCaptureCalibRequest.setBufferId(captureSettings.getStreamCaptureBufferID());
                        streamCaptureCalibRequest.setFilesize(String.valueOf(calibYmls.length));
                        streamCaptureCalibRequest.setFrameType(FRAME_TYPE_CALIZIP);
                        String streamCaptureCalibRequestStr = gson.toJson(streamCaptureCalibRequest);
                        _networkService.sendBinaryToHost(streamCaptureCalibRequestStr, calibYmls, true);
                        LogService.d(TAG,"send calibration zip to host " + streamCaptureCalibRequestStr);

                        byte[] landmark = DiskService.readFileToByte(ARC1_FACE_LANDMARk_FOLDER_PATH + "/facelandmark.yml");

                        if(landmark != null) {
                            StreamCaptureRequest streamCaptureRequest = new StreamCaptureRequest();
                            streamCaptureRequest.setRequest(captureSettings.getRequest());
                            streamCaptureRequest.setRequestId(captureSettings.getRequestID());
                            streamCaptureRequest.setBufferId(captureSettings.getStreamCaptureBufferID());
                            streamCaptureRequest.setFrameType(FRAME_TYPE_FACELM);
                            streamCaptureRequest.setFilesize(String.valueOf(landmark.length));

                            String streamCaptureRequestStr = gson.toJson(streamCaptureRequest);
                            LogService.i(TAG, "send faceLandmark to host " + streamCaptureRequestStr);
                            _networkService.sendBinaryToHost(streamCaptureRequestStr, landmark, true);
                        } else {
                            _errorReportingService.sendErrorToHost(new B3D4ClientError(
                                    B3D4ClientError.B3D_FIND_LANDMARK_ERROR.LANDMARKFILE_NOT_FOUND.getValue()),captureSettings);
                        }
                    }

                    /* we need to count color , IRL, IRR , so is done index is last three */
                    boolean isDone = (_streamStatistics.bufferDepthRawIndex == captureSettings.getDepthFrames());
                    LogService.v(TAG," _streamStatistics.bufferDepthRawIndex : " + _streamStatistics.bufferDepthRawIndex + " getDepthFrames : " + captureSettings.getDepthFrames());
                    if(isDone) stopClientProtect(); // stream_capture

                    /* use ARC_8M_TIMESTAMP_IRL_START - ARC_8M_COLOR_SIZE because the buffer only copy from L buffer
                     * there is no M buffer here
                     *  */
                    long timeStampL = ByteBuffer.wrap((FormattingService.byteArrayReverse
                            (Arrays.copyOfRange(byteData, ARC_8M_TIMESTAMP_IRL_START - ARC_8M_COLOR_SIZE, ARC_8M_TIMESTAMP_IRR_START - ARC_8M_COLOR_SIZE)))).getLong();
                    long timeStampR = ByteBuffer.wrap((FormattingService.byteArrayReverse
                            (Arrays.copyOfRange(byteData, ARC_8M_TIMESTAMP_IRR_START - ARC_8M_COLOR_SIZE, ARC_8M_TIMESTAMP_IRR_END - ARC_8M_COLOR_SIZE)))).getLong();

                    String fileNameL = String.format("%03d",_streamStatistics.bufferDepthRawIndex) +
                            "_"+String.format("%06d",timeStampL / NSTOMS) + "_" + String.format("%05d",_streamStatistics.bufferDepthRawIndex);
                    String fileNameR = String.format("%03d",_streamStatistics.bufferDepthRawIndex) +
                            "_"+String.format("%06d",timeStampR / NSTOMS) + "_" + String.format("%05d",_streamStatistics.bufferDepthRawIndex);

                    StreamCaptureRequest streamCaptureRequest = new StreamCaptureRequest();
                    streamCaptureRequest.setRequest(captureSettings.getRequest());
                    streamCaptureRequest.setRequestId(captureSettings.getRequestID());
                    streamCaptureRequest.setBufferId(captureSettings.getStreamCaptureBufferID());
                    streamCaptureRequest.setFrameType(FRAME_TYPE_STEREO);
                    streamCaptureRequest.setFilesize(String.valueOf(byteData.length));
                    streamCaptureRequest.setFilenameLIR(fileNameL);
                    streamCaptureRequest.setFilenameRIR(fileNameR);
                    streamCaptureRequest.setFrameIndex(String.valueOf(_streamStatistics.bufferDepthRawIndex));
                    streamCaptureRequest.setBufferFinish((isDone ? STRING_TRUE : STRING_FALSE));

                    String streamCaptureRequestStr = gson.toJson(streamCaptureRequest);
                    _networkService.sendBinaryToHost(streamCaptureRequestStr, byteData, true);
                    LogService.d(TAG,"send stereo image to host " + streamCaptureRequestStr);
                    break;
                }
                case STRING_BUFFER_FETCH :{
                    long timeStamp;
                    byte[] byteToSend;
                    String fileName;
                    String isFinish = (frameID == captureSettings.getBufferFetchSize()-1) ? STRING_TRUE : STRING_FALSE;
                    final int fetchFrameType = _mCaptureSettings.getBufferFetchType();

                    // color frame
                    if((BUFFER_FETCH_TYPE_COLOR & fetchFrameType) > 0) {
                        timeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                (Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_COLOR_START, ARC_2M_TIMESTAMP_IRL_START)))).getLong();
                        byteToSend = Arrays.copyOfRange(byteData, 0, ARC_2M_COLOR_SIZE);
                        fileName = String.format("%03d", frameID) +
                                "_" + String.format("%06d", timeStamp / NSTOMS) + "_" + String.format("%05d", frameID);

                        BufferFetchRequest bufferFetchColorRequest = new BufferFetchRequest();
                        bufferFetchColorRequest.setRequestId(captureSettings.getRequestID());
                        bufferFetchColorRequest.setRequest(captureSettings.getRequest());
                        bufferFetchColorRequest.setFrameType(FRAME_TYPE_COLOR);
                        bufferFetchColorRequest.setFileNameM(fileName);
                        bufferFetchColorRequest.setTimeStamp(String.valueOf(timeStamp));
                        bufferFetchColorRequest.setFileSize(String.valueOf(byteToSend.length));
                        bufferFetchColorRequest.setBufferId(captureSettings.getFetchBufferID());
                        bufferFetchColorRequest.setBufferFinish(isFinish);

                        String bufferFetchColorStr = gson.toJson(bufferFetchColorRequest);
                        _networkService.sendBinaryToHost(bufferFetchColorStr, byteToSend, true);
                        LogService.d(TAG, "send bufferFetch color image to host " + bufferFetchColorStr);
                    }

                    // depth frame
                    if((BUFFER_FETCH_TYPE_DEPTH & fetchFrameType) > 0) {
                        byte[] stereoFrame = Arrays.copyOfRange(byteData, ARC_2M_COLOR_SIZE, ARC_2M_TIMESTAMP_COLOR_START).clone();
                        byteToSend = _processingService.StartSingleDepthComputation(stereoFrame, ARC_2M_IR_WIDTH, ARC_2M_IR_HEIGHT, 0.75f);
                        // use IRL's timeStamp as depth's timeStamp
                        timeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                (Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_IRL_START, ARC_2M_TIMESTAMP_IRR_START)))).getLong();
                        fileName = String.format("%03d", frameID) +
                                "_" + String.format("%06d", timeStamp / NSTOMS) + "_" + String.format("%05d", frameID);

                        BufferFetchRequest bufferFetchDepthRequest = new BufferFetchRequest();
                        bufferFetchDepthRequest.setRequestId(captureSettings.getRequestID());
                        bufferFetchDepthRequest.setRequest(captureSettings.getRequest());
                        bufferFetchDepthRequest.setFrameType(FRAME_TYPE_DEPTH);
                        bufferFetchDepthRequest.setFileNameD(fileName);
                        bufferFetchDepthRequest.setTimeStamp(String.valueOf(timeStamp));
                        bufferFetchDepthRequest.setFileSize(String.valueOf(byteToSend.length));
                        bufferFetchDepthRequest.setBufferId(captureSettings.getFetchBufferID());
                        bufferFetchDepthRequest.setBufferFinish(isFinish);

                        String bufferFetchDepthStr = gson.toJson(bufferFetchDepthRequest);
                        _networkService.sendBinaryToHost(bufferFetchDepthStr, byteToSend, true);
                        LogService.d(TAG, "send bufferFetch depth image to host " + bufferFetchDepthStr);
                    }

                    // IRL frame
                    if((BUFFER_FETCH_TYPE_IRL & fetchFrameType) > 0) {
                        timeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                (Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_IRL_START, ARC_2M_TIMESTAMP_IRR_START)))).getLong();
                        byteToSend = Arrays.copyOfRange(byteData, ARC_2M_COLOR_SIZE, ARC_2M_COLOR_SIZE + ARC_2M_IR_SIZE);
                        fileName = String.format("%03d", frameID) +
                                "_" + String.format("%06d", timeStamp / NSTOMS) + "_" + String.format("%05d", frameID);

                        BufferFetchRequest bufferFetchIRLRequest = new BufferFetchRequest();
                        bufferFetchIRLRequest.setRequestId(captureSettings.getRequestID());
                        bufferFetchIRLRequest.setRequest(captureSettings.getRequest());
                        bufferFetchIRLRequest.setFrameType(FRAME_TYPE_IRL);
                        bufferFetchIRLRequest.setFileNameL(fileName);
                        bufferFetchIRLRequest.setTimeStamp(String.valueOf(timeStamp));
                        bufferFetchIRLRequest.setFileSize(String.valueOf(byteToSend.length));
                        bufferFetchIRLRequest.setBufferId(captureSettings.getFetchBufferID());
                        bufferFetchIRLRequest.setBufferFinish(isFinish);

                        String bufferFetchIRLStr = gson.toJson(bufferFetchIRLRequest);
                        _networkService.sendBinaryToHost(bufferFetchIRLStr, byteToSend, true);
                        LogService.d(TAG, "send bufferFetch IRL image to host " + bufferFetchIRLStr);
                    }

                    // IRR frame
                    if((BUFFER_FETCH_TYPE_IRR & fetchFrameType) > 0) {
                        timeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse
                                (Arrays.copyOfRange(byteData, ARC_2M_TIMESTAMP_IRR_START, ARC_2M_TIMESTAMP_IRR_END)))).getLong();
                        byteToSend = Arrays.copyOfRange(byteData, ARC_2M_COLOR_SIZE + ARC_2M_IR_SIZE, ARC_2M_COLOR_SIZE + ARC_2M_IR_SIZE * 2);
                        fileName = String.format("%03d", frameID) +
                                "_" + String.format("%06d", timeStamp / NSTOMS) + "_" + String.format("%05d", frameID);

                        BufferFetchRequest bufferFetchIRRRequest = new BufferFetchRequest();
                        bufferFetchIRRRequest.setRequestId(captureSettings.getRequestID());
                        bufferFetchIRRRequest.setRequest(captureSettings.getRequest());
                        bufferFetchIRRRequest.setFrameType(FRAME_TYPE_IRR);
                        bufferFetchIRRRequest.setFileNameR(fileName);
                        bufferFetchIRRRequest.setTimeStamp(String.valueOf(timeStamp));
                        bufferFetchIRRRequest.setFileSize(String.valueOf(byteToSend.length));
                        bufferFetchIRRRequest.setBufferId(captureSettings.getFetchBufferID());
                        bufferFetchIRRRequest.setBufferFinish(isFinish);

                        String bufferFetchIRRStr = gson.toJson(bufferFetchIRRRequest);
                        _networkService.sendBinaryToHost(bufferFetchIRRStr, byteToSend, true);
                        LogService.d(TAG, "send bufferFetch IRR image to host " + bufferFetchIRRStr);
                    }
                    break;
                }
                case STRING_STREAM_DEPTH:{
                    break;
                }
                case STRING_BUFFER_SNAPSHOT: {
                    break;
                }
            }
        }

        @Override
        public void onFrameNative(byte[] byteData, long mTotalFramesStreamed, long currentTimeStamp, int mResolution, boolean isCapture) {
            if(!_curHostCommands.equals(CMD_CANCEL_PROCESS))
                _processingService.addFrameToNativeJNI(byteData.clone(),mTotalFramesStreamed,currentTimeStamp, mResolution,isCapture);
        }
    };

    private final DepthCameraObserver mRecordingServiceObserver  = new DepthCameraObserver() {

        @Override
        public void onUpdate(DepthCameraState.StateType depthCameraState) {
            _mDepthCameraState = depthCameraState;
        }

        @Override
        public void onError(DepthCameraError depthCameraError) {

        }
    };

    /* a listener that listened native processor result */
    private final  B3D4NativeProcessorListener b3D4NativeProcessorListener = new B3D4NativeProcessorListener() {

        @Override
        public void onError(B3D4ClientError error, B3DCaptureSettings captureSetting) {
            _errorReportingService.sendErrorToHost(error,captureSetting);
            String request = captureSetting.getRequest().toLowerCase();
            LogService.d(TAG,"current request : " + request);
            if( request.equals(STRING_BUFFER_CAPTURE)    ||
                    request.equals(STRING_BUFFER_MERGE)  ||
                    request.equals(STRING_CANCEL_PROCESS)) {
                // reset curHostCommand when native error occur
                resetCurCommand();
            }
        }

        @Override
        public void onFrameEnough() {

        }

        @Override
        public void onProcessDone(byte[] bData) throws IOException {
            // check buffer to release
            if(_recordingService != null)
                _recordingService.checkMMapUsage();
        }

        @Override
        public void onFaceDetectDone(float[] headPoseInfo) {

        }

        public void onGenFaceLandMarkDone() {
            LogService.d(TAG, "onGenFaceLandMarkDone");
            _processingService.StopProcessJNI();
        }

        public void onRecalibrationDone(int errorcode, float calibDispErr) {
            if(_mCaptureSettings.getRequest().toLowerCase().equals(STRING_RECALIBRATION))
                stopClientProtect();
        }

        public void onStreamDepthDone(byte[] bData, long timeStampL) {
            _processingService.StopProcessJNI();
        }

        public void onProcessFinished(String who) {
            String curRequest = _mCaptureSettings.getRequest().toLowerCase();
            LogService.i(TAG,"finishing : " + who + ", curRequest : " + curRequest + ", preRequest : " + _prevHostCommands);
            if(curRequest.equals(STRING_CANCEL_PROCESS)) {

                if(_prevHostCommands.equals(CMD_STREAM_DEPTH)) {
                    // avoid case that cancel process with previous is stream_depth
                    // but native report process finish NOT streamdepth
                    if(who.toLowerCase().contains("streamdepth")) {
                        LogService.i(TAG, "native process " + who.toLowerCase() + " done on " + _curHostCommands);
                    } else {
                        LogService.i(TAG, "ignore " + who.toLowerCase() + " on " + STRING_CANCEL_PROCESS);
                        return;
                    }
                } else if (_prevHostCommands.equals(CMD_BUFFER_MERGE)) {
                    // avoid case that cancel process with previous is buffer_merge
                    // but native report process finish NOT dodepth or stitcher
                    if((who.toLowerCase().contains("dodepthwork")) ||
                        (who.toLowerCase().contains("dostitcherwork"))
                    ) {
                        LogService.i(TAG, "native process " + who.toLowerCase() + " done on " + _curHostCommands);
                    } else {
                        LogService.i(TAG, "ignore " + who.toLowerCase() + " on " + STRING_CANCEL_PROCESS);
                        return;
                    }
                }

                // clear all the native memory when cancel processing here
                _processingService.StopProcessJNI();

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        long startTime = System.currentTimeMillis();
                        // _mDepthCameraState is CONNECTED, it means sensor is closed, so can return buffer_cancel ok
                        // and add a timeout 5 s to avoid infinity loop
                        while (!((_mDepthCameraState == DepthCameraState.StateType.CONNECTED) ||
                                (System.currentTimeMillis() - startTime) > 5000)) {
                            LogService.i(TAG, "mDepthCameraState : " + _mDepthCameraState.getName());
                            try {
                                Thread.sleep(30);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                                LogService.logStackTrace(TAG, e.getStackTrace());
                            }
                        }

                        LogService.d(TAG, "done checking cancel condition");
                        Gson gson = new Gson();
                        String status = _mDepthCameraState == DepthCameraState.StateType.CONNECTED ? NetWorkMessage.Status.OK.getName()
                                : NetWorkMessage.Status.ERROR.getName();

                        AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTED);
                        CancelProcessResponseTo cancelProcessResponseTo = new CancelProcessResponseTo();
                        cancelProcessResponseTo.setStatus(status);
                        cancelProcessResponseTo.setResponseTo(_mCaptureSettings.getRequest());
                        cancelProcessResponseTo.setRequestId(_mCaptureSettings.getRequestID());

                        cmdLock.lock();
                        NetWorkMessage.HostCommands curCmd = _curHostCommands;
                        cmdLock.unlock();
                        if(!curCmd.equals(CMD_END)) {
                            String cancelProcessMsg = gson.toJson(cancelProcessResponseTo);
                            LogService.i(TAG, "send cancel process message to host : " + cancelProcessMsg);
                            _networkService.sendMessage(cancelProcessMsg);
                            // reset curCommands when cancel processing
                            resetCurCommand();
                        }
                    }}).start();
            } else if (curRequest.equals(STRING_BUFFER_MERGE)   ||
                       curRequest.equals(STRING_STREAM_DEPTH)) {
                // if native processing without cancel process, reset _curHostCommands here
                cmdLock.lock();
                // check _curHostCommands again to avoid the case
                // 1.receive buffer_merge,  _curHostCommands = CMD_BUFFER_MERGE
                // 2.receive buffer_cancel, _curHostCommands = CMD_CANCEL_PROCESS and native process call onProcessFinished
                //   arrive, it will change the _curHostCommands to CMD_END and cause problem
                NetWorkMessage.HostCommands curCmd = _curHostCommands;
                if(curCmd.equals(CMD_CANCEL_PROCESS)) {
                    LogService.w(TAG,"interrupt by cancel processing");
                } else {
                    // case without CMD_CANCEL_PROCESS
                    if(who.toLowerCase().contains("stitcher")) {
                        // CMD_BUFFER_MERGE
                        // contains doDepth and dostitcher, reset it at stitcher
                        // receive
                        _curHostCommands = CMD_END;
                    } else if(who.toLowerCase().contains("dodepth")){ //TODO rename native process name to sync with java side
                        // CMD_BUFFER_MERGE
                        // do nothing now
                    } else if(who.toLowerCase().contains("streamdepth")){
                        // CMD_STREAM_DEPTH
                        _curHostCommands = CMD_END;
                    }
                }
                cmdLock.unlock();
            }

            // check buffer to release
            if(_recordingService != null)
                _recordingService.checkMMapUsage();
        }
    };

    private TextureView.SurfaceTextureListener mSurfaceTextureListener = new TextureView.SurfaceTextureListener() {
        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            if (mCamLTexture.isAvailable() && mCamRTexture.isAvailable() && mCamMTexture.isAvailable()) {
                _recordingService.setPreviewTexture(mCamMTexture);
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

    private final B3DNetworkingListener mNetWorkingListener = new B3DNetworkingListener() {

        @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
        @Override
        public void onMessage(NetWorkMessage netMsg) {
            Gson gson = new Gson();
            // we need this capture setting for buffer_release
            // since this my be a specical case for current flow
            B3DCaptureSettings mCaptureSettings2 = null;
            boolean interrupted = false;

            cmdLock.lock();
            if(netMsg == null || netMsg.cmds == null ) {
                cmdLock.unlock();
                return;
            }
            else
                LogService.i(TAG, "incoming : " + netMsg.cmds.toString());

            if(netMsg.cmds.equals(CMD_BUFFER_RELEASE)) {
                LogService.i(TAG, "start running (interrupted) " + netMsg.cmds);
                mCaptureSettings2 = _recordingService.getCaptureSettings();
                mCaptureSettings2.set(netMsg);
                interrupted = true;
                cmdLock.unlock();
            } else {
                // start check request check
                // cancel process will reject all the commands
                if (_curHostCommands == CMD_CANCEL_PROCESS) {
                    LogService.w(TAG, "reject " + netMsg.cmds + " while " + _curHostCommands + " is running");
                    _errorReportingService.sendErrorToHost(new B3D4ClientError(SENSOR_IS_CANCELING.getValue()), netMsg);
                    cmdLock.unlock();
                    return;
                }

                // when doing buffer_merge and stream_depth, only accept cancel_processing
                if (_curHostCommands == CMD_BUFFER_MERGE || _curHostCommands == CMD_STREAM_DEPTH) {
                    if (!netMsg.cmds.equals(CMD_CANCEL_PROCESS)) {
                        LogService.w(TAG, "reject " + netMsg.cmds + " while " + _curHostCommands + " is running");
                        _errorReportingService.sendErrorToHost(new B3D4ClientError(SENSOR_IS_PROCESSING.getValue()), netMsg);
                        cmdLock.unlock();
                        return;
                    }
                }

                // finally accept incoming request
                _prevHostCommands = _curHostCommands;
                _curHostCommands = netMsg.cmds;
                LogService.i(TAG, "start running " + _curHostCommands);
                cmdLock.unlock();
            }

            if(interrupted) {
                if(mCaptureSettings2 == null) {
                    LogService.w(TAG,"mCaptureSettings2 is null.....");
                    return;
                }

                String request = mCaptureSettings2.getRequest().toLowerCase();
                if(request.equals(STRING_BUFFER_RELEASE)) {
                    _recordingService.runBufferRelease(mCaptureSettings2);
                    _recordingService.checkMMapUsage();
                } else {
                    LogService.w(TAG,"not supported interrupted command " + request);
                }
            } else {
                /* if camera is not in state CONNECTED return busy
                 * do not call _recordingService.setCaptureSettings here
                 * if camera is not ready and call set capture setting, it will change
                 * settings and cause some error/crash.
                 */
                if ((_curHostCommands.equals(CMD_BUFFER_CAPTURE) ||
                        _curHostCommands.equals(CMD_STREAM_CAPTURE) ||
                        _curHostCommands.equals(CMD_START_STREAM) ||
                        _curHostCommands.equals(CMD_BUFFER_MERGE) ||
                        _curHostCommands.equals(CMD_RECALIBRATION) ||
                        _curHostCommands.equals(CMD_STREAM_DEPTH)) &&
                        _mDepthCameraState != DepthCameraState.StateType.CONNECTED) {
                    _errorReportingService.sendErrorToHost(new B3D4ClientError(SENSOR_IS_BUSY.getValue()), netMsg);
                    // buffer_merge will update curHostCommands, so need to reset it here
                    resetCurCommand();
                    return;
                }

                if (_curHostCommands.equals(CMD_BUFFER_SNAPSHOT) || _curHostCommands.equals(CMD_BUFFER_SNAPSHOT_RECEIVE)) {
                    // if curHostCommand is CMD_BUFFER_SNAPSHOT or CMD_BUFFER_SNAPSHOT_RECEIVE, only update captureSetting
                    // but need to check current capture setting is STRING_STREAM_START or not
                    if (_mCaptureSettings != null && !_mCaptureSettings.getRequest().equals(STRING_STREAM_START)) {
                        _errorReportingService.sendErrorToHost(new B3D4ClientError(
                                B3D4ClientError.B3D_BUFFER_SNAP_ERROR.CURRENT_STREAM_NOT_RUNNING.getValue()), _mCaptureSettings);
                        return;
                    }
                    _mCaptureSettings.update(netMsg);
                } else {
                    /* new a capture setting and translate net message to capture settings */
                    _mCaptureSettings = _recordingService.getCaptureSettings();
                    _mCaptureSettings.set(netMsg);
                }

                switch (_curHostCommands) {
                    case CMD_CAMERA_CONFIGURATION: {
                        DeviceConfigResponseTo deviceConfigResponseTo = new DeviceConfigResponseTo();

                        deviceConfigResponseTo.setStatus(NetWorkMessage.Status.OK.getName());
                        deviceConfigResponseTo.setResponseTo(_mCaptureSettings.getRequest());
                        deviceConfigResponseTo.setRequestId(_mCaptureSettings.getRequestID());

                        String configs = DiskService.sdcardLoadFile(GlobalResourceService.CONFIG_FILE_PATH);
                        DeviceConfig deviceConfig = gson.fromJson(configs, DeviceConfig.class);
                        deviceConfigResponseTo.setConfiguration(deviceConfig);

                        String deviceConfigResponseToMsg = gson.toJson(deviceConfigResponseTo);
                        LogService.d(TAG, deviceConfigResponseToMsg);
                        _networkService.sendMessage(deviceConfigResponseToMsg);
                        break;
                    }
                    case CMD_CAPTURE_COMPRESSION: {
                        break;
                    }
                    case CMD_LAYOUT_POSITION: {
                        _deviceConfig.setLayoutName(netMsg.layout);
                        _deviceConfig.setLayoutDevices(Integer.valueOf(netMsg.layout_devices));
                        _deviceConfig.setLayoutPosition(netMsg.layout_position);
                        break;
                    }
                    case CMD_BUFFER_CAPTURE: {
                        if(_mCaptureSettings.getCaptureBufferID() != null)
                            checkValidSessionFolder(_mCaptureSettings.getCaptureBufferID());
                        CameraConfigService.resetExpWhiteBalenceValue();
                        if(_mCaptureSettings.getBufferCaptureMode().equals(BUFFER_CAPTURE_4D)) {
                            OffsetNotification offSetNotification = new OffsetNotification();
                            offSetNotification.setNotification(BUFFER_CAPTURE_DIFF);
                            offSetNotification.setBufferId(_mCaptureSettings.getCaptureBufferID());
                            offSetNotification.setRequestId(_mCaptureSettings.getRequestID());
                            String offsetNotificationStr = gson.toJson(offSetNotification);
                            _networkService.sendMessage(offsetNotificationStr);
                            LogService.i(TAG,"send notification to host " + offsetNotificationStr);
                        }
                        /* pass netMsg to recordingService, if we switch depthcamera in recordingService, we need to reconstruct capturesetting */
                        DiskService.createDirectory(new File(ARC_MMAP_FOLDER_PATH));
                        DiskService.createDirectory(new File(ARC_STILL_MMAP_FOLDER_PATH));
                        _recordingService.setCaptureSettings(_mCaptureSettings);
                        AppStatusManager.getInstance().setState(CAMERA_RECORDING);
                        startClientProtect(_mCaptureSettings.getRequest());
                        _recordingService.runBufferCapture(_mCaptureSettings, netMsg);
                        break;
                    }
                    case CMD_STREAM_CAPTURE: {
                        DiskService.createDirectory(new File(ARC1_FOLDER_PATH));
                        DiskService.createDirectory(new File(ARC1_FACE_LANDMARk_FOLDER_PATH));
                        DiskService.createDirectory(new File(ARC_MMAP_FOLDER_PATH));
                        DiskService.createDirectory(new File(ARC_ONE_MMAP_FOLDER_PATH));
                        LocalConfigService.setArc1LandMarkPathJNI(ARC1_FACE_LANDMARk_FOLDER_PATH);

                        _streamStatistics = new StreamHelper();
                        _recordingService.setCaptureSettings(_mCaptureSettings);
                        startClientProtect(_mCaptureSettings.getRequest());
                        _mCaptureSettings = _recordingService.runStreamCapture(_mCaptureSettings, netMsg);
                        AppStatusManager.getInstance().setState(CAMERA_RECORDING);
                        break;
                    }
                    case CMD_BUFFER_SNAPSHOT: {
                        if (_mDepthCameraState != DepthCameraState.StateType.STREAMING) {
                            _errorReportingService.sendErrorToHost(new B3D4ClientError(
                                    B3D4ClientError.B3D_BUFFER_SNAP_ERROR.CURRENT_STREAM_NOT_RUNNING.getValue()), _mCaptureSettings);
                            return;
                        }
                        _streamStatistics.bufferSnapShotIndex = 0;
                        _recordingService.runBufferSnapshot();
                        break;
                    }
                    case CMD_BUFFER_SNAPSHOT_RECEIVE: {
                        int index = 0;
                        File[] zipfiles = new File[4];
                        String[] paths = new String[4];
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/leftCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/midCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/rightCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/b3dCalibData.bin");

                        byte[] calibYmls = null;
                        calibYmls = DiskService.zipFiles(paths, zipfiles);
                        SnapShotReceiveRequest bufferSnapCalibRequest = new SnapShotReceiveRequest();
                        bufferSnapCalibRequest.setRequestId(_mCaptureSettings.getSubRequestID());
                        bufferSnapCalibRequest.setBufferId(_mCaptureSettings.getBufferSnapReceiveBufferID());
                        bufferSnapCalibRequest.setRequest(_mCaptureSettings.getSubRequest());
                        bufferSnapCalibRequest.setPosition(_mCaptureSettings.getBufferSnapCamPose());
                        bufferSnapCalibRequest.setRequestType(REQUEST_TYPE_SNAPSHOT);
                        bufferSnapCalibRequest.setFileSize(String.valueOf(calibYmls.length));
                        bufferSnapCalibRequest.setFrameType(FRAME_TYPE_CALIZIP);
                        String streamCaptureCalibRequestStr = gson.toJson(bufferSnapCalibRequest);
                        _networkService.sendBinaryToHost(streamCaptureCalibRequestStr, calibYmls, true);
                        LogService.d(TAG, "send calibration zip to host " + streamCaptureCalibRequestStr);

                        _recordingService.runBufferSnapShotReceive(_mCaptureSettings);
                        break;
                    }
                    case CMD_BUFFER_COLOR: {
                        AppStatusManager.getInstance().setState(CAMERA_PROCESSING_OR_TRANSFER);
                        List<Integer> target = _mCaptureSettings.getSelectedFrameIndices();
                        _recordingService.sendSelectedColorBuffer(_mCaptureSettings, target);
                        break;
                    }
                    case CMD_BUFFER_CANCEL: {
                        //TODO merge _recordingService.runCancelStream and _recordingService.cancelStream
                        stopClientProtect(); // stream_capture
                        _recordingService.runCancelStream(_mCaptureSettings);
                        _processingService.cancelProcessing();
                        String responsToStreamCapture = STRING_STREAM_CAPTURE;
                        StreamCaptureResponse streamCaptureResponse = new StreamCaptureResponse();
                        streamCaptureResponse.setResponseTo(responsToStreamCapture);
                        streamCaptureResponse.setRequestId(_mCaptureSettings.getRequestID());
                        streamCaptureResponse.setStatus(NetWorkMessage.Status.OK.getName());
                        String streamCaptureResponseStr = gson.toJson(streamCaptureResponse);
                        _networkService.sendMessage(streamCaptureResponseStr);
                        LogService.i(TAG, "send response to host " + streamCaptureResponseStr);

                        AppStatusManager.getInstance().setState(CAMERA_PROCESSING_OR_TRANSFER);
                        _recordingService.sendFirstThreeColorBuffer(_mCaptureSettings);
                        break;
                    }
                    case CMD_STREAM_DEPTH: {
                        DiskService.createDirectory(new File(ARC_MMAP_FOLDER_PATH));
                        DiskService.createDirectory(new File(ARC_MOTION_MMAP_FOLDER_PATH));
                        _streamStatistics = new StreamHelper();
                        if (_mCaptureSettings.getStreamDepthStartIndex() > 0)
                            _streamStatistics.StreamDepthDepthIndex = _mCaptureSettings.getStreamDepthStartIndex() - 1;
                        if (_mCaptureSettings.isDebug())
                            DiskService.createDirectory(new File(RECALIBRATION_FOLDER_PATH));
                        _processingService.setProcessingSettings(_mCaptureSettings);
                        _processingService.setStreamHelper(_streamStatistics);
                        _processingService.setProcessingSessionFolder(SESSIONS_FOLDER_PATH + "/" + _mCaptureSettings.getStreamDepthBufferID());
                        _processingService.startStreamDepth();
                        _recordingService.setCaptureSettings(_mCaptureSettings);
                        startClientProtect(_mCaptureSettings.getRequest());
                        _mCaptureSettings = _recordingService.runStreamDepth(_mCaptureSettings, netMsg);
                        int index = 0;
                        File[] zipfiles = new File[4];
                        String[] paths = new String[4];
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/leftCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/midCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/rightCam.yml");
                        paths[index] = ARC_CLIENT_PATH;
                        zipfiles[index++] = new File("CalibrationFiles/b3dCalibData.bin");

                        byte[] calibYmls = null;
                        calibYmls = DiskService.zipFiles(paths, zipfiles);

                        StreamCaptureRequest streamCaptureCalibRequest = new StreamCaptureRequest();
                        streamCaptureCalibRequest.setRequest(_mCaptureSettings.getRequest());
                        streamCaptureCalibRequest.setRequestId(_mCaptureSettings.getRequestID());
                        streamCaptureCalibRequest.setBufferId(_mCaptureSettings.getStreamDepthBufferID());
                        streamCaptureCalibRequest.setFilesize(String.valueOf(calibYmls.length));
                        streamCaptureCalibRequest.setFrameType(FRAME_TYPE_CALIZIP);
                        String streamCaptureCalibRequestStr = gson.toJson(streamCaptureCalibRequest);
                        _networkService.sendBinaryToHost(streamCaptureCalibRequestStr, calibYmls, true);
                        LogService.d(TAG, "send calibration zip to host " + streamCaptureCalibRequestStr);
                        AppStatusManager.getInstance().setState(CAMERA_RECORDING);
                        break;
                    }
                    case CMD_BUFFER_FETCH: {
                        final int fetchFrameType = _mCaptureSettings.getBufferFetchType();
                        if((BUFFER_FETCH_TYPE_CALIB & fetchFrameType) > 0) {
                            int index = 0;
                            File[] zipfiles = new File[4];
                            String[] paths = new String[4];
                            paths[index] = ARC_CLIENT_PATH;
                            zipfiles[index++] = new File("CalibrationFiles/leftCam.yml");
                            paths[index] = ARC_CLIENT_PATH;
                            zipfiles[index++] = new File("CalibrationFiles/midCam.yml");
                            paths[index] = ARC_CLIENT_PATH;
                            zipfiles[index++] = new File("CalibrationFiles/rightCam.yml");
                            paths[index] = ARC_CLIENT_PATH;
                            zipfiles[index++] = new File("CalibrationFiles/b3dCalibData.bin");

                            byte[] calibYmls = null;
                            calibYmls = DiskService.zipFiles(paths, zipfiles);

                            BufferFetchRequest bufferFetchCalibRequest = new BufferFetchRequest();
                            bufferFetchCalibRequest.setRequest(_mCaptureSettings.getRequest());
                            bufferFetchCalibRequest.setRequestId(_mCaptureSettings.getRequestID());
                            bufferFetchCalibRequest.setBufferId(_mCaptureSettings.getFetchBufferID());
                            bufferFetchCalibRequest.setFileSize(String.valueOf(calibYmls.length));
                            bufferFetchCalibRequest.setFrameType(FRAME_TYPE_CALIZIP);
                            String bufferFetchCalibRequestStr = gson.toJson(bufferFetchCalibRequest);
                            _networkService.sendBinaryToHost(bufferFetchCalibRequestStr, calibYmls, true);
                            LogService.d(TAG, "send calibration zip to host " + bufferFetchCalibRequestStr);
                        }
                        _recordingService.runBufferFetch(_mCaptureSettings);
                        break;
                    }
                    case CMD_BUFFER_MERGE: {
                        MMFrameBuffer processBufferV = _recordingService.getCaptureBuffer(_mCaptureSettings);
                        if (processBufferV.isEmpty()) {
                            // reset curHostCommand when error occur
                            resetCurCommand();
                            return;
                        }
                        AppStatusManager.getInstance().setState(CAMERA_PROCESSING_OR_TRANSFER);
                        if (_mCaptureSettings.isDebug())
                            DiskService.createDirectory(new File(RECALIBRATION_FOLDER_PATH));
                        DiskService.createDirectory(new File(SESSIONS_FOLDER_PATH + "/" + _mCaptureSettings.getMergeBufferID()));
                        _processingService.setProcessingSessionFolder(SESSIONS_FOLDER_PATH + "/" + _mCaptureSettings.getMergeBufferID());
                        _processingService.setProcessingSettings(_mCaptureSettings);
                        _processingService.startProcessing(processBufferV, RESOLUTION_8M.ordinal());
                        break;
                    }
                    case CMD_RECALIBRATION: {
                        if (_mCaptureSettings.isDebug())
                            DiskService.createDirectory(new File(RECALIBRATION_FOLDER_PATH));
                        _recordingService.setCaptureSettings(_mCaptureSettings);
                        startClientProtect(_mCaptureSettings.getRequest());
                        _recordingService.runRecalibration(_mCaptureSettings, netMsg);
                        _processingService.setProcessingSettings(_mCaptureSettings);
                        _processingService.startProcessingRecalibration();
                        AppStatusManager.getInstance().setState(CAMERA_PROCESSING_OR_TRANSFER);
                        break;
                    }
                    case CMD_START_STREAM: {
                        if(_mCaptureSettings.isDiagnostic()) {
                            GlobalResourceService.checkdiagnosticFiles();
                        }
                        _streamStatistics = new StreamHelper();
                        _streamStatistics.requestedFPS = _mCaptureSettings.getFPS();
                        _streamStatistics.tsStreamStart = System.currentTimeMillis();

                        _recordingService.setCaptureSettings(_mCaptureSettings);
                        //startClientProtect(_mCaptureSettings.getRequest());
                        _mCaptureSettings = _recordingService.runStartStream(_mCaptureSettings, netMsg);
                        if (_mCaptureSettings != null && _mCaptureSettings.isTrackingFace()) {
                            _processingService.setProcessingSettings(_mCaptureSettings);
                            _processingService.startProcessingFaceDetection();
                        }
                        AppStatusManager.getInstance().setState(CAMERA_RECORDING);
                        break;
                    }
                    case CMD_STOP_STREAM: {
                        stopClientProtect(); // start_stream
                        _recordingService.setCaptureSettings(_mCaptureSettings);
                        _processingService.StopProcessJNI();
                        if (_mCaptureSettings.getStreamMode().toLowerCase().equals(STRING_STREAM_MODE_SELFSCAN)) {
                            DiskService.createDirectory(new File(ARC1_FOLDER_PATH));
                            DiskService.createDirectory(new File(ARC1_FACE_LANDMARk_FOLDER_PATH));
                            LocalConfigService.setArc1LandMarkPathJNI(ARC1_FACE_LANDMARk_FOLDER_PATH);
                            DiskService.deleteFile(ARC1_FACE_LANDMARk_FILE);
                            _recordingService.generatePreviewLandMark();
                        }
                        _recordingService.runStopStream(_streamStatistics, _mCaptureSettings);
                        _mCaptureSettings.setSize(_streamStatistics.streamWidth, _streamStatistics.streamHeight);
                        AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTED);
                        break;
                    }
                    case CMD_CANCEL_PROCESS: {
                        /* if previous command is buffer_merge or stream_depth, will need to cancel native processor
                         *  others like buffer_capture, only need to make sure camera is stop
                         * */
                        stopClientProtect(); // cancel processing
                        LogService.i(TAG, "prev request is : " + _prevHostCommands);
                        _recordingService.setCaptureSettings(_mCaptureSettings);
                        if (_prevHostCommands.equals(CMD_BUFFER_MERGE) || _prevHostCommands.equals(CMD_STREAM_DEPTH)) {
                            _processingService.setProcessingSettings(_mCaptureSettings);
                            _processingService.cancelProcessing();
                            _recordingService.cancelStream(_mCaptureSettings);
                        } else {
                            _recordingService.cancelStream(_mCaptureSettings);
                            AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTED);
                            CancelProcessResponseTo cancelProcessResponseTo = new CancelProcessResponseTo();
                            cancelProcessResponseTo.setStatus(NetWorkMessage.Status.OK.getName());
                            cancelProcessResponseTo.setResponseTo(_mCaptureSettings.getRequest());
                            cancelProcessResponseTo.setRequestId(_mCaptureSettings.getRequestID());

                            String cancelProcessMsg = gson.toJson(cancelProcessResponseTo);
                            LogService.i(TAG, "send cancel process message to host : " + cancelProcessMsg);
                            _networkService.sendMessage(cancelProcessMsg);
                            // reset curCommands when cancel processing
                            resetCurCommand();
                        }
                        break;
                    }
                    default: {
                        LogService.w(TAG, " not supported commands " + netMsg.cmds);
                    }
                }
            }
        }

        @Override
        public void onErrorRequest(NetWorkMessage.ErrorRequest request) {
            switch (request) {
                case REQUEST_STOP_STREAM : {
                    _processingService.StopProcessJNI();
                    _recordingService.runStopStream(_streamStatistics, _mCaptureSettings);
                    _mCaptureSettings.setSize(_streamStatistics.streamWidth, _streamStatistics.streamHeight);
                    AppStatusManager.getInstance().setState(WEBSOCKET_CONNECTED);
                    break;
                }
                case REQUEST_RESTART_CLIENT :
                    GlobalResourceService.restartClient();
                    break;
                case REQUEST_REBOOT :
                    GlobalResourceService.rebootDevice("Arc"); //TODO define reason
                    break;
            }
        }

        @Override
        public void onUpdateUI(String view, String message) {}

        @Override
        public void onUIEvent(String event, int value) { }
    };

    private void startClientProtect(String request) {
        String who = request + "_Protection";
        if(mClientProtectThread != null) mClientProtectThread.quit();
        mClientProtectThread = new HandlerThread(who);
        mClientProtectThread.start();
        mClientProtectHandler = new Handler(mClientProtectThread.getLooper());
        mClientProtectControl = true;
        mClientProtectHandler.post(processProtection);
    }

    private void stopClientProtect() {
        mClientProtectControl = false;
        if(mClientProtectThread != null) mClientProtectThread.quit();
    }

    private Runnable processProtection = new Runnable () {
        @Override
        public void run() {
            LogService.i(TAG,"start monitor");
            final long timeToStop = 120000;
            final long starTime = System.currentTimeMillis();
            while(mClientProtectControl) {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    LogService.e(TAG, e.toString());
                    e.printStackTrace();
                    LogService.logStackTrace(TAG, e.getStackTrace());
                }
                long curTimeStamp = System.currentTimeMillis();
                if((curTimeStamp - starTime) > timeToStop && _mDepthCameraState != DepthCameraState.StateType.CONNECTED) {
                    LogService.i(TAG,"after " + timeToStop/MSTOSEC + " sec, camera state : " + _mDepthCameraState +
                            ", stop the client!!!");
                    //GlobalResourceService.restartClient();
                    GlobalResourceService.killselfPid();
                }
            }

            LogService.i(TAG,"current camera state : " + _mDepthCameraState + ", stop monitor");
        }
    };

    boolean checkValidSessionFolder(String currentBufferID) {
        File target = new File(SESSIONS_FOLDER_PATH);
        File[] result = CheckUtil.sortFolderName(target);

        // a.compareTo(b)
        // a < b return -1, a > b return 1, a = b return 0
        // if we start from leng = 9, it should avoid file sync with monitor thread
        if (result != null && result.length > 9) {
            // if currentBufferID < latest one
            if (currentBufferID.compareTo(result[0].getName()) < 0) {
                // from oldest , if currentBufferID < result[x], delete the oldest one
                for (int x = result.length - 1; x >= 0; x--)
                    if (currentBufferID.compareTo(result[x].getName()) < 0) {
                        LogService.i(TAG, result[x].getName() + " is newer than " + currentBufferID + ", delete it");
                        File deletetarget = new File(result[x].getAbsolutePath());
                        if (deletetarget.isDirectory()) {
                            try {
                                FileUtils.deleteDirectory(deletetarget);
                            } catch (IOException e) {
                                e.printStackTrace();
                                LogService.e(TAG,"" + e.getMessage());
                                LogService.logStackTrace(TAG, e.getStackTrace());
                            }
                        }
                        break;
                    }
            }
        }
        return true;
    }
}

