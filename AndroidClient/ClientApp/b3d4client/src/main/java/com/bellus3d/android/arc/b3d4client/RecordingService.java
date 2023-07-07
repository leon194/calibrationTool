package com.bellus3d.android.arc.b3d4client;

import android.content.Context;
import android.graphics.ImageFormat;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.Pair;
import android.view.TextureView;

import com.bellus3d.android.arc.b3d4client.ConfigService.CameraConfigService;
import com.bellus3d.android.arc.b3d4client.ConfigService.LocalConfigService;
import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.JsonModel.*;
import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.*;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage;
import com.bellus3d.android.arc.b3d4client.NetworkService.NetworkService;
import com.bellus3d.android.arc.b3d4client.camera.B3DCaptureSettings;
import com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraError;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraObserver;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraState;
import com.bellus3d.android.arc.b3d4client.camera.DepthCameraStreamListener;
import com.google.gson.Gson;

import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import static com.bellus3d.android.arc.b3d4client.DiskService.removeDirectory;
import static com.bellus3d.android.arc.b3d4client.AppStatusManager.AppState.*;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_BUFFER_FETCH_ERROR.*;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_BUFFER_SNAPSHOT_RECEIVE_ERROR.*;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_SENSOR_STREAMING_ERROR.*;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_SINGLE_VIEW_MERGE_ERROR.*;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.*;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.CAMERAID.*;
import static com.bellus3d.android.arc.b3d4client.LogService.LogService.initNativeLogFile;
import static com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage.*;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.CaptureEvent.*;
import static com.bellus3d.android.arc.b3d4client.camera.B3DDepthCamera.TripleCamResolution.RESOLUTION_2M;

public class RecordingService {

    static final B3DDepthCamera.DeviceType DeviceCamType = B3DDepthCamera.DeviceType.DEVICE_B3D4;
    static B3DDepthCamera.Control FloodStatus = B3DDepthCamera.Control.CONTROL_OFF;
    static B3DDepthCamera.Control ProjectorStatus = B3DDepthCamera.Control.CONTROL_OFF;

    private B3DDepthCamera _depthCamera;
    private static TextureView _cameraTexture;
    private DepthCameraStreamListener _registrantListener = null ;
    private DepthCameraObserver _registrantObserver = null;
    private B3D4ClientError _b3D4ClientError;
    private Context activityContex;
    private NetworkService _networkService;
    private DeviceConfig _deviceConfig;

    private final int MaxBufferCacheMap = 15;
    private int MaxBufferCaptureFrame = 25;
    private final int _B3D_OK = B3D4ClientError.B3D_ERROR_CATEGORY.B3D_OK.getValue();
    private DepthCameraState.StateType _mDepthCameraState = DepthCameraState.StateType.CONNECTED;

    private Lock mmapCacheLock = new ReentrantLock();

    public Map<String, MMFrameBuffer > bufferCacheMap; // buffer_capture save buffer by buffer id
    MMFrameBuffer mmBufferCapture;
    public Map<String, Pair<String, String> > colorBufferMap; // map<Ltimestamp, <Mtimestamp,path >>
    private boolean _isCancelStream = false;
    public static enum BUFFER_CACHE_FLAG {
        FLAG_CAN_RELEASE("0"),
        FLAG_IS_PROCESSING("1");

        private final String flag;

        BUFFER_CACHE_FLAG(String flag) { this.flag = flag; }

        public String getValue() { return flag; }
    }

    public List<byte[]> snapShotBufferMap;   // buffer_snap shot save buffer by frame id, buffer_receive get send the buffer
    private byte[] Arc1PreviewFrame;
    long bufferSnapCount = 0;
    private static int BUFFER_SNAP_SKIP_COUNT = 5;
    private FPSChecker _fpsChecker = null;

    private Gson gson;

    public RecordingService() {
        bufferCacheMap = new HashMap<String, MMFrameBuffer >();
        mmBufferCapture = new MMFrameBuffer(15);
        _b3D4ClientError = new B3D4ClientError();
        gson = new Gson();
        colorBufferMap = new HashMap<String, Pair<String, String> >();
    }

    // must be called before initialize camera
    public void setPreviewTexture(TextureView texture){
        _cameraTexture = texture;
        if(_depthCamera != null && _depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
            _depthCamera.setPreviewTexture(_cameraTexture,null);
        }
    }

    public void setPreviewTarget(final TextureView viewL, final TextureView viewR, final TextureView viewM) {
        _depthCamera.setPreviewTarget(viewL,viewR,viewM);
    }

    public void registerStreamListener(DepthCameraStreamListener listener){
        _registrantListener = listener;
    }

    public void setDeviceConfig(DeviceConfig deviceConfig) {
        _deviceConfig = deviceConfig;
    }

    public void registerObserver(DepthCameraObserver observer){
        _registrantObserver = observer;
    }

    public void setCaptureEvent(B3DCaptureSettings settings, B3DDepthCamera.CaptureEvent captureEvent){
        _depthCamera.setCaptureEvent(settings,captureEvent);
    }

    public void initializeCamera(final Context context){
        activityContex = context;
        _depthCamera = new B3DDepthCamera(context,DeviceCamType);
        initNativeLogFile();
        _depthCamera.registerStreamListener(mDepthCameraStreamListener);
        _depthCamera.registerObserver(mDepthCameraObserver);
        _depthCamera.setCameraID(CAMERAID_RGB.getValue());
    }

    public B3DDepthCamera getDepthCamera() {
        return _depthCamera;
    }

    public B3DCaptureSettings getCaptureSettings() {
        return _depthCamera.getCaptureSettings();
    }

    public void setCaptureSettings(B3DCaptureSettings captureSettings){
        _depthCamera.setCaptureSettings(captureSettings);
    }


    public void setNetworkService(NetworkService networkService){
        _networkService = networkService;
    }

    public MMFrameBuffer getCaptureBuffer(B3DCaptureSettings captureSettings) {
        String buffer_id = null;
        final String request = captureSettings.getRequest();
        if(request.toLowerCase().equals(STRING_BUFFER_MERGE))
            buffer_id = captureSettings.getMergeBufferID();
        else if(request.toLowerCase().equals(STRING_BUFFER_FETCH))
            buffer_id = captureSettings.getFetchBufferID();

        if(bufferCacheMap == null || bufferCacheMap.size() <= 0) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return new MMFrameBuffer(0);
        }
        else if(!bufferCacheMap.containsKey(buffer_id)) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_ID_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return new MMFrameBuffer(0);
        }

        MMFrameBuffer frameBufferV = bufferCacheMap.get(buffer_id);

        if(frameBufferV.mmBufferV == null || frameBufferV.mmBufferV.size() == 0) {
            _b3D4ClientError.setErrorCode(MERGE_INPUT_CAPTURE_BUFFER_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return new MMFrameBuffer(0);
        }

        return frameBufferV;
    }

    public Map<String, Pair<String, String> > getColorBuffer() {
        if(colorBufferMap == null || colorBufferMap.size() <= 0) {
            LogService.e(TAG,"Invalid colorBufferMap");
            return new HashMap<String, Pair<String, String> >();
        }
        return colorBufferMap;
    }

    /* CMD_BUFFER_CAPTURE */
    /* buffer capture will create a new capture settings,too. but it will no need to set to processing service like stream start */
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public void runBufferCapture(B3DCaptureSettings captureSettings, NetWorkMessage netMsg) {
        if(activityContex == null ) {
            _b3D4ClientError.setErrorCode(ACTIVITY_CONTEXT_IS_NULL.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,"activityContex is null ");
            return;
        }
        if(captureSettings == null ) {
            _b3D4ClientError.setErrorCode(CAPTURE_SETTING_IS_NULL.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,"captureSettings is null ");
            return;
        }

        if(bufferCacheMap == null) {
            bufferCacheMap = new HashMap<String, MMFrameBuffer >();
        }

        MaxBufferCaptureFrame = captureSettings.getBufferFrames();

        _isCancelStream = false;

        if(captureSettings.getBufferFrames() > 0) {
            mmBufferCapture = null;
            mmBufferCapture = new MMFrameBuffer(captureSettings.getBufferFrames());
            // init capture buffer id for cancel process use
            mmBufferCapture.setFolderName(captureSettings.getCaptureBufferID());
        }

        if(bufferCacheMap.size() >= MaxBufferCacheMap) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_OVERFLOW.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.w(TAG,"buffer cache is full");
            return;
        }

        if (_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_FILES) &&
                captureSettings.getBufferFrames() == 0) {
            _b3D4ClientError = _depthCamera.setCaptureEvent(captureSettings, CAPTURE_SINGLE);
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettings);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return;
            }
            _b3D4ClientError = _depthCamera.startStreamSync();
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettings);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return;
            }
        } else if(captureSettings.getBufferFrames() == 0 && captureSettings.getBufferSource() == B3DCaptureSettings.CameraSource.COLOR) {
            // a special case for frames = 0 && source = color && DEVICE_B3D4_SINGLE case
            B3DCaptureSettings captureSettingsL = captureSettings;
            if(!_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4_SINGLE)) {
                _b3D4ClientError = _depthCamera.closeSync();
                if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                    reportError(_b3D4ClientError, captureSettings);
                    LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                    return;
                }
                _depthCamera.unregisterStreamListener(null);
                _depthCamera = null;
                _depthCamera = new B3DDepthCamera(activityContex,B3DDepthCamera.DeviceType.DEVICE_B3D4_SINGLE);
                initNativeLogFile();
                _depthCamera.registerStreamListener(mDepthCameraStreamListener);
                _depthCamera.registerObserver(mDepthCameraObserver);
                captureSettingsL = _depthCamera.getCaptureSettings();
                captureSettingsL.set(netMsg);
                _depthCamera.setCaptureSettings(captureSettingsL);
            }
            _b3D4ClientError = _depthCamera.setCaptureEvent(captureSettingsL, CAPTURE_SINGLE);
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettingsL);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return;
            }

            /* use rgb first, if there is a need to capture IRs add logic here */
            _depthCamera.setCameraID(CAMERAID_RGB.getValue());
            CameraConfigService.setCamera17FPS(30,0); //30 will cause network reject, but we can support 30FPS
            _b3D4ClientError = _depthCamera.openSync();
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettingsL);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return;
            }
            _b3D4ClientError = _depthCamera.startStreamSync();
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettingsL);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return;
            }

        } else {
            B3DCaptureSettings captureSettingsL = captureSettings;

            /* other buffer capture case uses camera id 17 */
            if(_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4) ||
                    _depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4_SINGLE)) {
                if(!_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
                    _b3D4ClientError = _depthCamera.closeSync();
                    if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                        reportError(_b3D4ClientError, captureSettingsL);
                        LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                        return;
                    }
                    _depthCamera.unregisterStreamListener(null);
                    _depthCamera = null;
                    _depthCamera = new B3DDepthCamera(activityContex,B3DDepthCamera.DeviceType.DEVICE_B3D4);
                    initNativeLogFile();
                    _depthCamera.registerStreamListener(mDepthCameraStreamListener);
                    _depthCamera.registerObserver(mDepthCameraObserver);
                    captureSettingsL = _depthCamera.getCaptureSettings();
                    captureSettingsL.set(netMsg);
                    _depthCamera.setCaptureSettings(captureSettingsL);
                }

                if (captureSettingsL.hasStatsColor()) {
                    int expline = captureSettings.getStatsColorExpLine();
                    int target_expline = expline;
                    if(captureSettingsL.getBufferCaptureMode().equals(BUFFER_CAPTURE_STILL))
                        target_expline = expline*EXP_LINE_TIME_8M/EXP_LINE_TIME_2M*OV8856_BINNING_FACTOR;
                    int gain = captureSettings.getStatsColorGainLuminocity();
                    int gainRed = captureSettings.getStatsColorGainRed();
                    int gainGreen = captureSettings.getStatsColorGainGreen();
                    int gainBlue = captureSettings.getStatsColorGainBlue();
                    LogService.i(TAG, "set properties "+
                            "  expline:" + expline +
                            ", target_expline:" + target_expline +
                            ", gain:" + gain +
                            ", gainRed:" + gainRed +
                            ", gainGreen:" + gainGreen +
                            ", gainBlue:" + gainBlue);

                    //String exp_gain = "0 100000 " + String.valueOf(gainLuminocity);
                    //CameraConfigService.set("persist.vendor.isp.ae.exp_gain", exp_gain);
                    CameraConfigService.set("persist.camera.ov8856.shutter", String.valueOf(target_expline));
                    CameraConfigService.set("persist.camera.ov8856.gain", String.valueOf(gain));
                    CameraConfigService.set("persist.vendor.camera.bypass.ae", String.valueOf(1));
                    CameraConfigService.set("persist.vendor.camera.bypass.awb", String.valueOf(1));
                    CameraConfigService.set("debug.isp.awb.rgain.set", String.valueOf(gainRed));
                    CameraConfigService.set("debug.isp.awb.ggain.set", String.valueOf(gainGreen));
                    CameraConfigService.set("debug.isp.awb.bgain.set", String.valueOf(gainBlue));
                }

                /* decide to capture low res or high res here
                *  if BufferCaptureMode not set, default use 8M
                * */
                if(captureSettingsL.getBufferCaptureMode().equals(BUFFER_CAPTURE_4D))
                    _b3D4ClientError = _depthCamera.setCaptureEvent(captureSettingsL, CAPTURE_BUFFER_2M); // 8M/2M
                else
                    _b3D4ClientError = _depthCamera.setCaptureEvent(captureSettingsL, CAPTURE_BUFFER_8M); // 8M/2M

                if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                    reportError(_b3D4ClientError, captureSettingsL);
                    LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                    return;
                }

                LogService.d(TAG, " set IR exp to " + captureSettings.getIrExp() +" ms");

                if(captureSettingsL.getBufferCaptureMode().equals(BUFFER_CAPTURE_4D))
                    CameraConfigService.setCamera17FPS(10, (long) (captureSettings.getIrExp() * 1000));
                else
                    CameraConfigService.setCamera17FPS(5,(long)(captureSettings.getIrExp()*1000));

                _depthCamera.setPreviewTexture(_cameraTexture,null);
                _b3D4ClientError = _depthCamera.openSync();
                if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                    reportError(_b3D4ClientError, captureSettingsL);
                    LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                    return;
                }
            }
            if(captureSettingsL.isIrProjectorOn())
                _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_ON,"255");
            if(captureSettingsL.isIrFloodOn())
                _depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_ON,"255");

            /* enable native start add frame flag before start stream */
            _depthCamera.setDecodeTypeJNI(B3DDepthCamera.DecodeType.SINGLEVIEW.ordinal());
            _b3D4ClientError = _depthCamera.startStreamSync();
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettingsL);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return;
            }
        }
    }

    public void runBufferFetch(B3DCaptureSettings captureSettings) {
        final String buffer_id = captureSettings.getFetchBufferID();

        if(bufferCacheMap == null || bufferCacheMap.size() <= 0) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
        else if(!bufferCacheMap.containsKey(buffer_id)) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_ID_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }

        MMFrameBuffer frameBufferV = bufferCacheMap.get(buffer_id);
        captureSettings.setBufferFetchSize(frameBufferV.mmBufferV.size());

        if(frameBufferV.mmBufferV == null || frameBufferV.mmBufferV.size() == 0) {
            _b3D4ClientError.setErrorCode(FETCH_INPUT_CAPTURE_BUFFER_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }

        for(int index = 0 ; index < frameBufferV.mmBufferV.size() ;  index++) {
            final String fileName = frameBufferV.mmBufferV.get(index).first;
            final long frameID = frameBufferV.mmBufferV.get(index).second.first;
            final byte[] bufferToSend = DiskService.mmapGet(fileName);
            if(bufferToSend != null && !captureSettings.getRequest().equals(STRING_CANCEL_PROCESS)) {
                if(_registrantListener != null)
                    _registrantListener.onFrameHost(captureSettings,bufferToSend,frameID);
                else
                    LogService.e(TAG,"_registrantListener is null on file " + fileName + ", frameID" + frameID);
            } else {
                LogService.w(TAG,"can't find file " + fileName + " or current request is  "
                        + captureSettings.getRequest().equals(STRING_CANCEL_PROCESS));
            }
        }
    }

    public void runBufferSnapshot(){
        LogService.i(TAG,"");
        _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_ON,"255");
        bufferSnapCount = 0;
        snapShotBufferMap = null;
        snapShotBufferMap = new ArrayList<>();
    }

    public void runBufferSnapShotReceive(B3DCaptureSettings captureSettings){
        if(snapShotBufferMap.isEmpty()) {
            _b3D4ClientError.setErrorCode(CURRENT_BUFFER_CACHE_EMPTY.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }

        for(int x = 0 ; x < snapShotBufferMap.size() ; x++)
            _registrantListener.onFrameHostSub(captureSettings, snapShotBufferMap.get(x), x);

        snapShotBufferMap.clear();
    }

    /* CMD_BUFFER_RELEASE */
    public void runBufferRelease(B3DCaptureSettings captureSettings) {
        if(bufferCacheMap == null){
            LogService.d(TAG,"buffer cache map is null, no-op");
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_INVALID.getValue());
            reporWarning(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
        if(captureSettings.getReleaseBufferID() == null ||
                !bufferCacheMap.containsKey(captureSettings.getReleaseBufferID())) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_ID_INVALID.getValue());
            reporWarning(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            AppStatusManager.getInstance().setState(AppStatusManager.AppState.CAMERA_PROCESSING_COMPLETED);
            return;
        }

        // only set a release flag to true
        LogService.d(TAG,"try to release id " + captureSettings.getReleaseBufferID());
        if(bufferCacheMap.containsKey(captureSettings.getReleaseBufferID())) {
            LogService.d(TAG,"set " + captureSettings.getReleaseBufferID() + " as true");
            bufferCacheMap.get(captureSettings.getReleaseBufferID()).setcanRelease(true);
        }

        //send success to host, add here first to make function work
        BufferIdResponseTo bufferIdResponseTo = new BufferIdResponseTo();
        bufferIdResponseTo.setStatus(Status.OK.getName());
        bufferIdResponseTo.setResponseTo(captureSettings.getRequest());
        bufferIdResponseTo.setRequestId(captureSettings.getRequestID());
        bufferIdResponseTo.setBufferId(captureSettings.getReleaseBufferID());
        String bufferIdResponseToMsg = gson.toJson(bufferIdResponseTo);
        LogService.i(TAG, bufferIdResponseToMsg);
        _networkService.sendMessage(bufferIdResponseToMsg);
        AppStatusManager.getInstance().setState(AppStatusManager.AppState.CAMERA_PROCESSING_COMPLETED);

    }

    /* CMD_RECALIBRATION */
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    /* if stream start will create a new capture setting, we need to find a way to return it to MainActivity */
    public void runRecalibration(B3DCaptureSettings captureSettings, NetWorkMessage netMsg){
        B3DCaptureSettings captureSettingsL = captureSettings;
        if(_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4_SINGLE) ||
                _depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
            if(!_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
                _b3D4ClientError = _depthCamera.closeSync();
                if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                    reportError(_b3D4ClientError, captureSettings);
                    LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                    return;
                }
                _depthCamera.unregisterStreamListener(null);
                _depthCamera = null;
                _depthCamera = new B3DDepthCamera(activityContex,B3DDepthCamera.DeviceType.DEVICE_B3D4);
                initNativeLogFile();
                _depthCamera.registerStreamListener(mDepthCameraStreamListener);
                _depthCamera.registerObserver(mDepthCameraObserver);
                captureSettingsL = _depthCamera.getCaptureSettings();
                captureSettingsL.set(netMsg);
                _depthCamera.setCaptureSettings(captureSettingsL);
            }

            CameraConfigService.setCamera17FPS(5,(long)(3.5*1000));
            _depthCamera.setPreviewTexture(_cameraTexture,null);
            _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_ON, "255");
            _b3D4ClientError = _depthCamera.openSync();
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettingsL);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return;
            }
        }

        _b3D4ClientError = _depthCamera.setCaptureEvent(captureSettingsL, CAPTURE_BUFFER_8M);
        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettingsL);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
        _depthCamera.setDecodeTypeJNI(B3DDepthCamera.DecodeType.RECALIBRATION.ordinal());
        _b3D4ClientError = _depthCamera.startStreamSync();
        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettingsL);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
    }

    /* CMD_STREAM_CAPTURE */
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    /* if stream start will create a new capture setting, we need to find a way to return it to MainActivity */
    public B3DCaptureSettings runStreamCapture(B3DCaptureSettings captureSettings, NetWorkMessage netMsg){

        colorBufferMap = null;
        colorBufferMap = new HashMap<String, Pair<String, String> >();
        removeDirectory(ARC_ONE_MMAP_FOLDER_PATH);

        B3DCaptureSettings captureSettingsL = captureSettings;
        if(_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4_SINGLE) ||
                _depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
            if(!_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
                _b3D4ClientError = _depthCamera.closeSync();
                if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                    reportError(_b3D4ClientError, captureSettings);
                    LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                    return captureSettings;
                }
                _depthCamera.unregisterStreamListener(null);
                _depthCamera = null;
                _depthCamera = new B3DDepthCamera(activityContex,B3DDepthCamera.DeviceType.DEVICE_B3D4);
                initNativeLogFile();
                _depthCamera.registerStreamListener(mDepthCameraStreamListener);
                _depthCamera.registerObserver(mDepthCameraObserver);
                captureSettingsL = _depthCamera.getCaptureSettings();
                captureSettingsL.set(netMsg);
                _depthCamera.setCaptureSettings(captureSettingsL);
            }

            if (captureSettingsL.hasStatsColor()) {
                int expline = captureSettings.getStatsColorExpLine();
                int target_expline = expline*EXP_LINE_TIME_8M/EXP_LINE_TIME_2M*OV8856_BINNING_FACTOR;
                int gain = captureSettings.getStatsColorGainLuminocity();
                int gainRed = captureSettings.getStatsColorGainRed();
                int gainGreen = captureSettings.getStatsColorGainGreen();
                int gainBlue = captureSettings.getStatsColorGainBlue();
                LogService.i(TAG, "set properties "+
                        "  expline:" + expline +
                        ", target_expline:" + target_expline +
                        ", gain :" + gain +
                        ", gainRed:" + gainRed +
                        ", gainGreen:" + gainGreen +
                        ", gainBlue:" + gainBlue);

                //String exp_gain = "0 100000 " + String.valueOf(gainLuminocity);
                //CameraConfigService.set("persist.vendor.isp.ae.exp_gain", exp_gain);
                CameraConfigService.set("persist.camera.ov8856.shutter", String.valueOf(target_expline));
                CameraConfigService.set("persist.camera.ov8856.gain", String.valueOf(gain));
                CameraConfigService.set("persist.vendor.camera.bypass.ae", String.valueOf(1));
                CameraConfigService.set("persist.vendor.camera.bypass.awb", String.valueOf(1));
                CameraConfigService.set("debug.isp.awb.rgain.set", String.valueOf(gainRed));
                CameraConfigService.set("debug.isp.awb.ggain.set", String.valueOf(gainGreen));
                CameraConfigService.set("debug.isp.awb.bgain.set", String.valueOf(gainBlue));
            }

            CameraConfigService.setCamera17FPS(5,(long)(9.5*1000));
            _depthCamera.setPreviewTexture(_cameraTexture,null);
            _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_ON, "255");
            _b3D4ClientError = _depthCamera.openSync();
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettingsL);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return captureSettingsL;
            }
        }

        /* we decide use 2M RGB resolution for ARC1 */
        _b3D4ClientError = _depthCamera.setCaptureEvent(captureSettingsL, CAPTURE_BUFFER_8M);

        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettingsL);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return captureSettingsL;
        }
        _depthCamera.setDecodeTypeJNI(B3DDepthCamera.DecodeType.FACELANDMARK.ordinal());
        _b3D4ClientError = _depthCamera.startStreamSync();
        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettingsL);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return captureSettingsL;
        }
        return captureSettingsL;
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public B3DCaptureSettings runStreamDepth(B3DCaptureSettings captureSettings, NetWorkMessage netMsg){

        colorBufferMap = null;
        colorBufferMap = new HashMap<String, Pair<String, String> >();
        removeDirectory(ARC_MOTION_MMAP_FOLDER_PATH);

        B3DCaptureSettings captureSettingsL = captureSettings;
        if(_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4_SINGLE) ||
                _depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
            if(!_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
                _b3D4ClientError = _depthCamera.closeSync();
                if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                    reportError(_b3D4ClientError, captureSettings);
                    LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                    return captureSettings;
                }
                _depthCamera.unregisterStreamListener(null);
                _depthCamera = null;
                _depthCamera = new B3DDepthCamera(activityContex,B3DDepthCamera.DeviceType.DEVICE_B3D4);
                initNativeLogFile();
                _depthCamera.registerStreamListener(mDepthCameraStreamListener);
                _depthCamera.registerObserver(mDepthCameraObserver);
                captureSettingsL = _depthCamera.getCaptureSettings();
                captureSettingsL.set(netMsg);
                _depthCamera.setCaptureSettings(captureSettingsL);
            }

            if (captureSettingsL.hasStatsColor()) {
                int expline = captureSettings.getStatsColorExpLine();
                int target_expline = expline*EXP_LINE_TIME_8M/EXP_LINE_TIME_2M*OV8856_BINNING_FACTOR;
                int gain = captureSettings.getStatsColorGainLuminocity();
                int gainRed = captureSettings.getStatsColorGainRed();
                int gainGreen = captureSettings.getStatsColorGainGreen();
                int gainBlue = captureSettings.getStatsColorGainBlue();
                LogService.i(TAG, "set properties "+
                        "  expline:" + expline +
                        ", target_expline:" + target_expline +
                        ", gain :" + gain +
                        ", gainRed:" + gainRed +
                        ", gainGreen:" + gainGreen +
                        ", gainBlue:" + gainBlue);

                //String exp_gain = "0 100000 " + String.valueOf(gainLuminocity);
                //CameraConfigService.set("persist.vendor.isp.ae.exp_gain", exp_gain);
                CameraConfigService.set("persist.camera.ov8856.shutter", String.valueOf(target_expline));
                CameraConfigService.set("persist.camera.ov8856.gain", String.valueOf(gain));
                CameraConfigService.set("persist.vendor.camera.bypass.ae", String.valueOf(1));
                CameraConfigService.set("persist.vendor.camera.bypass.awb", String.valueOf(1));
                CameraConfigService.set("debug.isp.awb.rgain.set", String.valueOf(gainRed));
                CameraConfigService.set("debug.isp.awb.ggain.set", String.valueOf(gainGreen));
                CameraConfigService.set("debug.isp.awb.bgain.set", String.valueOf(gainBlue));
            }
            LogService.i(TAG, " set IR exp to " + captureSettings.getIrExp() +" ms");
            CameraConfigService.setCamera17FPS(5,(long)(captureSettings.getIrExp()*1000));
            _depthCamera.setPreviewTexture(_cameraTexture,null);
            _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_ON, "255");
            _b3D4ClientError = _depthCamera.openSync();
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettingsL);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return captureSettingsL;
            }
        }

        _b3D4ClientError = _depthCamera.setCaptureEvent(captureSettingsL, CAPTURE_BUFFER_8M);

        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettingsL);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return captureSettingsL;
        }
        _depthCamera.setDecodeTypeJNI(B3DDepthCamera.DecodeType.DEPTHCOMPUTATION.ordinal());
        _b3D4ClientError = _depthCamera.startStreamSync();
        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettingsL);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return captureSettingsL;
        }
        return captureSettingsL;
    }

    /* CMD_STREAM_START */
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    /* if stream start will create a new capture setting, we need to find a way to return it to MainActivity */
    public B3DCaptureSettings runStartStream(B3DCaptureSettings captureSettings, NetWorkMessage netMsg){
        B3DCaptureSettings captureSettingsL = captureSettings;
        if(_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4_SINGLE) ||
                _depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
            if(!_depthCamera.getDepthCamType().equals(B3DDepthCamera.DeviceType.DEVICE_B3D4)) {
                _b3D4ClientError = _depthCamera.closeSync();
                if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                    reportError(_b3D4ClientError, captureSettings);
                    LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                    return captureSettings;
                }
                _depthCamera.unregisterStreamListener(null);
                _depthCamera = null;
                _depthCamera = new B3DDepthCamera(activityContex,B3DDepthCamera.DeviceType.DEVICE_B3D4);
                initNativeLogFile();
                _depthCamera.registerStreamListener(mDepthCameraStreamListener);
                _depthCamera.registerObserver(mDepthCameraObserver);
                captureSettingsL = _depthCamera.getCaptureSettings();
                captureSettingsL.set(netMsg);
                _depthCamera.setCaptureSettings(captureSettingsL);
            }

            LogService.i(TAG, " set IR exp to " + captureSettings.getIrExp() +" ms");
            CameraConfigService.setCamera17FPS(20,(long)(captureSettings.getIrExp()*1000));
            _depthCamera.setPreviewTexture(_cameraTexture,null);


            if(captureSettingsL.isTrackingFace() && !captureSettingsL.getStreamMode().toLowerCase().equals(STRING_STREAM_MODE_OPERATORSCAN)) {
                _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_ON, "255");
                captureSettingsL.setTrackingFace(null);
            }

            _b3D4ClientError = _depthCamera.openSync();
            if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
                reportError(_b3D4ClientError, captureSettingsL);
                LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
                return captureSettingsL;
            }
        }

        _b3D4ClientError = _depthCamera.setCaptureEvent(captureSettingsL, CAPTURE_BUFFER_2M);
        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettingsL);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return captureSettingsL;
        }

        if(captureSettingsL.getStreamMode().toLowerCase().equals(STRING_STREAM_MODE_SELFSCAN)) {
            Arc1PreviewFrame = null;
            Arc1PreviewFrame = new byte[ARC_2M_COLOR_SIZE];
        }

        if(captureSettingsL.isDiagnostic())
            _depthCamera.setDecodeTypeJNI(B3DDepthCamera.DecodeType.DIAGNOSTIC.ordinal());
        else
            _depthCamera.setDecodeTypeJNI(B3DDepthCamera.DecodeType.FACEDETECTION.ordinal());
        _b3D4ClientError = _depthCamera.startStreamSync();
        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettingsL);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return captureSettingsL;
        }

        return captureSettingsL;
    }

    /* CMD_STREAM_STOP */
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public void runStopStream(StreamHelper streamStatistics, B3DCaptureSettings captureSettings){
        LogService.d(TAG, "");
        B3D4ClientError b3D4ClientError = _depthCamera.stopStreamSync();
        if( b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
        b3D4ClientError = _depthCamera.closeSync();
        if( b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }

        _depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_OFF,"0");
        _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF,"0");

        long captureMS = _depthCamera.getCaptureMilliseconds();
        long processMS = _depthCamera.getProcessMilliseconds();

        String now = TimeStampService.getTimestamp(null, GlobalResourceService.TIME_DIFFERENCE);
        long totalMS = (System.currentTimeMillis() - streamStatistics.tsStreamStart);

        StreamStopResponseTo streamStopResponseTo = new StreamStopResponseTo();
        streamStopResponseTo.setResponseTo(NetWorkMessage.ResponseTo.STREAM_STOP.getName());
        streamStopResponseTo.setRequestId(captureSettings.getRequestID());
        streamStopResponseTo.setWidth(String.valueOf(streamStatistics.streamWidth));
        streamStopResponseTo.setHeight(String.valueOf(streamStatistics.streamHeight));
        streamStopResponseTo.setStreamedFrames(String.valueOf(streamStatistics.framesStreamed));
        streamStopResponseTo.setStreamedBytes(String.valueOf(streamStatistics.streamedBytes));
        streamStopResponseTo.setCapturedFrames(String.valueOf(streamStatistics.framesCaptured));
        streamStopResponseTo.setCaptureMs(String.valueOf(captureMS));
        streamStopResponseTo.setTotalMs(String.valueOf(totalMS));

        int gainArray[] = LocalConfigService.GetLastAEAWBValueJNI();
        if (gainArray[0] != 0 || gainArray[1] != 0 || gainArray[2] != 0 || gainArray[3] != 0 || gainArray[4] != 0 || gainArray[5] != 0) {
            Color color = new Color();
            color.setExposureLine(String.valueOf(gainArray[0]));
            color.setExposureTime(String.valueOf(gainArray[2]/NSTOMS));
            color.setY(String.valueOf(gainArray[1]));
            color.setR(String.valueOf(gainArray[3]));
            color.setG(String.valueOf(gainArray[4]));
            color.setB(String.valueOf(gainArray[5]));

            StreamStats streamStats = new StreamStats();
            streamStats.setColor(color);
            streamStopResponseTo.setStreamStats(streamStats);

        }

        String streamStopResponseToMsg = gson.toJson(streamStopResponseTo);
        LogService.i(TAG, streamStopResponseToMsg);
        _networkService.sendMessage(streamStopResponseToMsg);
    }

    public void runCancelStream(B3DCaptureSettings captureSettings){
        LogService.v(TAG, "");
        B3D4ClientError b3D4ClientError = _depthCamera.stopStreamSync();
        if( b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
        b3D4ClientError = _depthCamera.closeSync();
        if( b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }

        _depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_OFF,"0");
        _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF,"0");
    }

    public void updateDepthMapSetting(){}

    public void updateBufferCacheMapFlag(String key, BUFFER_CACHE_FLAG flag, boolean value) {
        if(bufferCacheMap != null)
            LogService.d(TAG,"bufferCacheMap, key : " + key+ "is contain : " + bufferCacheMap.containsKey(key)  + " flag : " + flag);
        if(bufferCacheMap != null && bufferCacheMap.containsKey(key)) {
            switch (flag) {
                case FLAG_CAN_RELEASE:
                    bufferCacheMap.get(key).setcanRelease(value);
                    break;
                case FLAG_IS_PROCESSING:
                    bufferCacheMap.get(key).setisProcessing(value);
                    break;
                default:
                    LogService.w(TAG,"not a support flag" + flag);
            }
        }
    }

    private final DepthCameraStreamListener mDepthCameraStreamListener = new DepthCameraStreamListener() {

        @Override
        public void onWarning(B3D4ClientError error, B3DCaptureSettings captureSetting) {

        }

        @Override
        public void onError(B3D4ClientError error, B3DCaptureSettings captureSetting) {
            reportError(error,captureSetting);
        }

        @RequiresApi(api = Build.VERSION_CODES.N)
        @Override
        public void onFrameBuffer(B3DCaptureSettings mCaptureSetting, FrameBuffer frameBuffer) {
        }

        public void onFrameBytes(B3DCaptureSettings mCaptureSetting, byte[] byteData) {
            synchronized (Arc1PreviewFrame) {
                Arc1PreviewFrame = byteData.clone();
            }
        }

        @Override
        public void onStatusHost(B3DCaptureSettings mCaptureSetting, String status) {
            if(_registrantListener != null)
                _registrantListener.onStatusHost(mCaptureSetting, status);
        }

        @Override
        public void onMsgHost(B3DCaptureSettings mCaptureSetting, String msg) {
            if(_registrantListener != null)
                _registrantListener.onMsgHost(mCaptureSetting,msg);
        }

        @Override
        public void onNotificationHost(B3DCaptureSettings mCaptureSetting, long timeStampL, long timeStampR, long frameIndex) {
            if(_registrantListener != null)
                _registrantListener.onNotificationHost( mCaptureSetting,timeStampL,timeStampR,frameIndex);
        }

        @Override
        public void onFrameHost(final B3DCaptureSettings mCaptureSetting, final byte[] byteData, final long frameID) {
            if(mCaptureSetting.getRequest().equals(STRING_STREAM_CAPTURE) && _registrantListener != null) {
                if(colorBufferMap != null) {
                    long colorTimeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                            Arrays.copyOfRange(byteData, ARC_8M_TIMESTAMP_COLOR_START, ARC_8M_TIMESTAMP_IRL_START)))).getLong() / 1000000;
                    long irLTimeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                            Arrays.copyOfRange(byteData, ARC_8M_TIMESTAMP_IRL_START, ARC_8M_TIMESTAMP_IRR_START)))).getLong() / 1000000;
                    byte[] colorbyte = Arrays.copyOfRange(byteData, 0, ARC_8M_COLOR_SIZE).clone();
                    String filename = ARC_ONE_MMAP_FOLDER_PATH + "/" + colorTimeStamp;
                    if(DiskService.mmapSave(colorbyte, filename) && colorBufferMap != null) {
                        colorBufferMap.put(String.valueOf(irLTimeStamp), new Pair<String, String>(String.valueOf(colorTimeStamp), filename));
                    } else {
                        LogService.w(TAG, filename + " not added!");
                    }
                    _registrantListener.onFrameHost(mCaptureSetting,Arrays.copyOfRange(byteData, ARC_8M_COLOR_SIZE, ARC_8M_TIMESTAMP_IRR_END),frameID);
                } else {
                    LogService.e(TAG,"color Map is null");
                }
            } else if(mCaptureSetting.getRequest().equals(STRING_BUFFER_CAPTURE) && _registrantListener != null) {
                if(frameID == 0 && _registrantListener != null )
                    _registrantListener.onFrameHost(mCaptureSetting,byteData,frameID);

                // if buffer_capture with buffer frames = 0, no need to cache it
                if(mCaptureSetting.getBufferFrames() > 0) {
                    String bufferid = mCaptureSetting.getCaptureBufferID();
                    String sessionFolderName = ARC_STILL_MMAP_FOLDER_PATH + "/" + bufferid;
                    DiskService.createDirectory(new File(sessionFolderName));

                    long timeStamp = 0L;
                    if(mCaptureSetting.getBufferCaptureMode().equals(BUFFER_CAPTURE_STILL))
                        timeStamp= ByteBuffer.wrap((FormattingService.byteArrayReverse(
                                Arrays.copyOfRange(byteData, ARC_8M_COLOR_SIZE + ARC_8M_IR_SIZE * 2, ARC_8M_COLOR_SIZE + ARC_8M_IR_SIZE * 2 + ARC_8M_TIMESTAMP_SIZE)))).getLong() / 1000000;
                    else if(mCaptureSetting.getBufferCaptureMode().equals(BUFFER_CAPTURE_4D))
                        timeStamp= ByteBuffer.wrap((FormattingService.byteArrayReverse(
                                Arrays.copyOfRange(byteData, ARC_2M_COLOR_SIZE + ARC_2M_IR_SIZE * 2, ARC_2M_COLOR_SIZE + ARC_2M_IR_SIZE * 2 + ARC_2M_TIMESTAMP_SIZE)))).getLong() / 1000000;

                    String filename = sessionFolderName + "/" + String.valueOf(timeStamp);

                    if (!_isCancelStream && DiskService.mmapSave(byteData, filename)) {
                        mmBufferCapture.mmBufferV.add(new Pair<String, Pair<Long, Long>>(filename, new Pair<Long, Long>(frameID, timeStamp)));
                    } else {
                        if(_isCancelStream)
                            LogService.i(TAG,"process canceled, stop add frame");
                        else
                            LogService.w(TAG, filename + " not added!");
                    }

                    // we get enough buffers, update the session buffer to cache map
                    if (mmBufferCapture.mmBufferV.size() == mCaptureSetting.getBufferFrames() && !_isCancelStream) {
                        if (bufferCacheMap.containsKey(mCaptureSetting.getCaptureBufferID()))
                            bufferCacheMap.replace(mCaptureSetting.getCaptureBufferID(), mmBufferCapture);
                        else
                            bufferCacheMap.put(mCaptureSetting.getCaptureBufferID(), mmBufferCapture);

                        // finally we get all the buffer to cache map, send ok to host
                        _registrantListener.onStatusHost(mCaptureSetting, NetWorkMessage.Status.OK.getName());
                    }
                }
            } else if(mCaptureSetting.getRequest().equals(STRING_STREAM_DEPTH) && _registrantListener != null) {
                if(colorBufferMap != null && _registrantListener != null) {
                    final long colorTimeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                            Arrays.copyOfRange(byteData, ARC_8M_TIMESTAMP_COLOR_START, ARC_8M_TIMESTAMP_IRL_START)))).getLong() / 1000000;
                    long irLTimeStamp = ByteBuffer.wrap((FormattingService.byteArrayReverse(
                            Arrays.copyOfRange(byteData, ARC_8M_TIMESTAMP_IRL_START, ARC_8M_TIMESTAMP_IRR_START)))).getLong() / 1000000;
                    String fileNameColor = String.format("%03d",frameID + 1) +
                            "_"+colorTimeStamp + "_" + String.format("%05d",frameID + 1);
                    StreamCaptureRequest streamDepthColorTimeStampRequest = new StreamCaptureRequest();
                    streamDepthColorTimeStampRequest.setRequestId(mCaptureSetting.getRequestID());
                    streamDepthColorTimeStampRequest.setRequest(mCaptureSetting.getRequest());
                    streamDepthColorTimeStampRequest.setBufferId(mCaptureSetting.getStreamDepthBufferID());
                    streamDepthColorTimeStampRequest.setFrameType(FRAME_TYPE_COLOR_NOTIFICATION);
                    streamDepthColorTimeStampRequest.setFilenameColor(fileNameColor);

                    String streamDepthTimeStampRequestStr = gson.toJson(streamDepthColorTimeStampRequest);
                    byte[] dummy = streamDepthTimeStampRequestStr.getBytes();
                    _networkService.sendBinaryToHost(streamDepthTimeStampRequestStr,dummy,true);
                    LogService.d(TAG, "send color timestamp to host " + streamDepthTimeStampRequestStr);

                    if(frameID == 0) { // send first color back to host
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                int frameIdx = ( mCaptureSetting.getStreamDepthStartIndex() > 0 ) ? mCaptureSetting.getStreamDepthStartIndex() : 1;
                                String fileNameColor = String.format("%03d",frameIdx) +
                                        "_"+String.format("%06d",colorTimeStamp) + "_" + String.format("%05d",frameIdx);
                                byte[] jpgResult =
                                        FormattingService.compressYuvImg(Arrays.copyOfRange(byteData, 0, ARC_8M_COLOR_SIZE).clone(),
                                                ImageFormat.NV21, ARC_8M_COLOR_WIDTH, ARC_8M_COLOR_HEIGHT, null, "JPEG");

                                BufferColorRequest bufferColorRequest = new BufferColorRequest();
                                bufferColorRequest.setRequest(mCaptureSetting.getRequest());
                                bufferColorRequest.setRequestId(mCaptureSetting.getRequestID());
                                bufferColorRequest.setBufferId(mCaptureSetting.getStreamDepthBufferID());
                                bufferColorRequest.setFilesize(String.valueOf(jpgResult.length));
                                bufferColorRequest.setFilenameColor(fileNameColor);
                                bufferColorRequest.setRequestType(mCaptureSetting.getBufferColorRequestType());
                                bufferColorRequest.setFrameType("jpg");
                                bufferColorRequest.setDeviceId(GlobalResourceService.getDeviceId());
                                bufferColorRequest.setCamera(_deviceConfig.getLayoutPosition());

                                String bufferColorRequestStr = gson.toJson(bufferColorRequest);
                                if (!AppStatusManager.getInstance().getState().equals(CAMERA_RECORDING)) {
                                    AppStatusManager.getInstance().setState(CAMERA_PROCESSING_OR_TRANSFER);
                                }
                                _networkService.sendBinaryToHost(bufferColorRequestStr, jpgResult, true);
                            }
                        }).start();
                    }

                    byte[] colorbyte = Arrays.copyOfRange(byteData, 0, ARC_8M_COLOR_SIZE).clone();
                    String filename = ARC_MOTION_MMAP_FOLDER_PATH + "/" + colorTimeStamp;
                    if(DiskService.mmapSave(colorbyte, filename) && colorBufferMap != null ) {
                        colorBufferMap.put(String.valueOf(irLTimeStamp), new Pair<String, String>(String.valueOf(colorTimeStamp), filename));
                    } else {
                        LogService.w(TAG, filename + " not added!");
                    }
                } else {
                    LogService.e(TAG,"color Map is null");
                }
            }  else if(mCaptureSetting.getRequest().equals(STRING_STREAM_START) && _registrantListener != null) {
                final String subaction = mCaptureSetting.getSubRequest().toLowerCase();

                if(mCaptureSetting.isTrackingFace() && byteData != null)
                    _registrantListener.onFrameNative(byteData.clone(), frameID,
                            0, RESOLUTION_2M.ordinal(), false);
                // keep one copy of preview frame, when stop_stream, use this to generate a color space faceLandMark
                if(mCaptureSetting.getStreamMode().toLowerCase().equals(STRING_STREAM_MODE_SELFSCAN)) {
                    synchronized (Arc1PreviewFrame) {
                        Arc1PreviewFrame = Arrays.copyOfRange(byteData.clone(), 0,ARC_2M_COLOR_SIZE).clone();
                    }
                }

                if(subaction.equals(STRING_BUFFER_SNAPSHOT) &&
                        mCaptureSetting.getSubRequestID() != null &&
                        bufferSnapCount < mCaptureSetting.getBufferSnapFrames() + BUFFER_SNAP_SKIP_COUNT) {

                    if( bufferSnapCount == 0) {
                        _fpsChecker = null;
                        _fpsChecker = new FPSChecker(mCaptureSetting.getBufferSnapFramesFPS(), System.currentTimeMillis());
                    }
                    if(bufferSnapCount < BUFFER_SNAP_SKIP_COUNT) {
                        // skip these frames(BUFFER_SNAP_SKIP_COUNT), these frames might have no projector on it
                        LogService.v(TAG,"skip first " + BUFFER_SNAP_SKIP_COUNT +
                                " frame for projector not ready, current is" + bufferSnapCount);
                        bufferSnapCount++;
                    } else {
                        if(_fpsChecker != null && _fpsChecker.isValid(System.currentTimeMillis()) || _fpsChecker == null) {
                            if (snapShotBufferMap.size() < mCaptureSetting.getBufferSnapFrames()) {
                                snapShotBufferMap.add(byteData.clone());
                                LogService.d(TAG, "snapShotBufferMap : " + snapShotBufferMap.size());
                                bufferSnapCount++;
                                if (snapShotBufferMap.size() == mCaptureSetting.getBufferSnapFrames()) {
                                    _registrantListener.onStatusHostSub(mCaptureSetting, NetWorkMessage.Status.OK.getName());
                                    _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF, "0");
                                }
                            }
                        }
                        LogService.v(TAG,"skip");
                    }
                }

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        // preview only use 2M resolution
                        int scaleRatio = ARC_2M_COLOR_WIDTH / mCaptureSetting.getWidth();
                        try {
                            mCaptureSetting.setSize(mCaptureSetting.getWidth(), mCaptureSetting.getHeight());
                            byte[] scaledResult =
                                    FormattingService.compressYuvImg(FormattingService.scaleYUV420(Arrays.copyOfRange(byteData, 0, ARC_2M_COLOR_SIZE),
                                            ARC_2M_COLOR_WIDTH, ARC_2M_COLOR_HEIGHT, scaleRatio), ImageFormat.NV21, mCaptureSetting.getWidth(), mCaptureSetting.getHeight(), null, "JPEG");
                            _registrantListener.onFrameHost(mCaptureSetting, scaledResult, frameID);
                        } catch (Exception e) {
                            if (mCaptureSetting == null)
                                LogService.e(TAG, " mCaptureSettings is null" + e.toString());
                            else
                                LogService.e(TAG, e.toString());
                            e.printStackTrace();
                            LogService.logStackTrace(TAG, e.getStackTrace());
                        }
                    }
                }).start();
            } else if (_registrantListener != null)
                _registrantListener.onFrameHost(mCaptureSetting,byteData.clone(),frameID);
        }

        @Override
        public void onFrameNative(byte[] byteData, long frameCount, long timestamp, int isValid, boolean isCapture) {
            if(_registrantListener != null)
                _registrantListener.onFrameNative(byteData,frameCount,timestamp,isValid,isCapture);
        }
    };

    private final DepthCameraObserver mDepthCameraObserver  = new DepthCameraObserver() {

        @Override
        public void onUpdate(DepthCameraState.StateType depthCameraState) {
            _mDepthCameraState = depthCameraState;
            if(_registrantObserver != null)
                _registrantObserver.onUpdate(depthCameraState);
        }

        @Override
        public void onError(DepthCameraError depthCameraError) {
            if(_registrantObserver != null)
                _registrantObserver.onError(depthCameraError);
        }
    };

    private void reporWarning(B3D4ClientError error, B3DCaptureSettings captureSetting) {
        if(_registrantListener != null)
            _registrantListener.onWarning(error,captureSetting);

        _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF,"0");
        _depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_OFF,"0");
    }

    private void reportError(B3D4ClientError error, B3DCaptureSettings captureSetting) {
        if(_registrantListener != null)
            _registrantListener.onError(error,captureSetting);

        _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF,"0");
        _depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_OFF,"0");
    }

    public void sendFirstThreeColorBuffer(B3DCaptureSettings captureSettings) {
        if(colorBufferMap == null || colorBufferMap.isEmpty()) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
        ArrayList<String> list = new ArrayList<>(colorBufferMap.keySet());
        Collections.sort(list, new Comparator<String>() {
            @Override
            public int compare(String o1, String o2) {
                return Long.parseLong(o1)>Long.parseLong(o2)?1:-1;
            }
        });

        for (int x = 0 ; x < 3 ; x++) {
            String fileNameColor = String.format("%03d", (x * 2) + 1) +
                    "_" + String.format("%06d", Long.parseLong(colorBufferMap.get(list.get(x*2)).first, 10)) + "_" + String.format("%05d", (x * 2) + 1);

            String request = captureSettings.getRequest().toLowerCase();
            StreamCaptureRequest streamCaptureColorRequest = new StreamCaptureRequest();
            if(captureSettings.getRequest().toLowerCase().equals(STRING_BUFFER_CANCEL))
                streamCaptureColorRequest.setRequest(STRING_STREAM_CAPTURE);
            else
                streamCaptureColorRequest.setRequest(captureSettings.getRequest());
            streamCaptureColorRequest.setRequestId(captureSettings.getRequestID());
            switch (request) {
                case STRING_BUFFER_CANCEL : {
                    streamCaptureColorRequest.setBufferId(captureSettings.getBufferCancelBufferID());
                    break;
                }
                case STRING_STREAM_CAPTURE: {
                    streamCaptureColorRequest.setBufferId(captureSettings.getStreamCaptureBufferID());
                    break;
                }
                default:
                    streamCaptureColorRequest.setBufferId(captureSettings.getStreamCaptureBufferID());
                    LogService.e(TAG," request " + request + " not supported");
            }

            String filename = colorBufferMap.get(list.get(x*2)).second;
            byte[] bufferToSend = DiskService.mmapGet(filename);
            String fileLength = String.valueOf(0);

            if(bufferToSend == null)
                LogService.w(TAG," file not exist : " + filename);
            else
                fileLength = String.valueOf(bufferToSend.length);

            streamCaptureColorRequest.setFrameType(FRAME_TYPE_COLOR);
            streamCaptureColorRequest.setFilesize(fileLength);
            streamCaptureColorRequest.setFilenameColor(fileNameColor);
            streamCaptureColorRequest.setFrameIndex(String.valueOf(x + 1));
            streamCaptureColorRequest.setBufferFinish(((x == 2) ? STRING_TRUE : STRING_FALSE));

            String streamCaptureColorRequestStr = gson.toJson(streamCaptureColorRequest);
            _networkService.sendBinaryToHost(streamCaptureColorRequestStr, bufferToSend, true);
            LogService.i(TAG, "send color image to host " + streamCaptureColorRequestStr);
        }
    }

    public void sendSelectedColorBuffer(B3DCaptureSettings captureSettings, List<Integer> target) {
        if(colorBufferMap == null || colorBufferMap.isEmpty()) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
        ArrayList<String> list = new ArrayList<>(colorBufferMap.keySet());
        Collections.sort(list, new Comparator<String>() {
            @Override
            public int compare(String o1, String o2) {
                return Long.parseLong(o1)>Long.parseLong(o2)?1:-1;
            }
        });


        for (int x = 0 ; x < target.size() ; x ++) {

            if(target.get(x) < 0 ) continue;
            int targetIndex = -1;
            String fileNameColor = "null";
            byte[] bufferToSend = null;
            String requesType = captureSettings.getBufferColorRequestType().toLowerCase();
            switch (requesType) {
                case STRING_BUFFER_COLOR_ARC1 : {
                    targetIndex = target.get(x);
                    break;
                }
                case STRING_BUFFER_COLOR_ARCX : {
                    targetIndex = target.get(x) > 0 ? target.get(x) - 1 : target.get(x);
                    break;
                }
            }

            LogService.v(TAG,"target.get(x) : " + target.get(x) + " targetIndex : " + targetIndex);

            String fileLength = String.valueOf(0);

            switch (requesType) {
                case STRING_BUFFER_COLOR_ARC1 : {
                    fileNameColor = String.format("%03d",target.get(x) + 1) +
                            "_"+String.format("%06d",Long.parseLong(colorBufferMap.get(list.get(targetIndex)).first,10)) + "_" + String.format("%05d",target.get(x) + 1);

                    String filename = colorBufferMap.get(list.get(targetIndex)).second;
                    bufferToSend = DiskService.mmapGet(filename);
                    if(bufferToSend == null)
                        LogService.w(TAG," file not exist : " + filename);
                    else
                        fileLength = String.valueOf(bufferToSend.length);
                    break;
                }
                case STRING_BUFFER_COLOR_ARCX : {
                    fileNameColor = String.format("%03d",target.get(x)) +
                            "_"+String.format("%06d",Long.parseLong(colorBufferMap.get(list.get(targetIndex)).first,10)) + "_" + String.format("%05d",target.get(x));
                    String filename = colorBufferMap.get(list.get(targetIndex)).second;
                    bufferToSend =
                            FormattingService.compressYuvImg(DiskService.mmapGet(filename),
                                    ImageFormat.NV21, ARC_8M_COLOR_WIDTH, ARC_8M_COLOR_HEIGHT, null, "JPEG");
                    if(bufferToSend == null)
                        LogService.w(TAG," file not exist : " + filename);
                    else
                        fileLength = String.valueOf(bufferToSend.length);
                    break;
                }
            }

            BufferColorRequest bufferColorRequest = new BufferColorRequest();
            bufferColorRequest.setRequest(captureSettings.getRequest());
            bufferColorRequest.setRequestId(captureSettings.getRequestID());
            bufferColorRequest.setBufferId(captureSettings.getBufferColorBufferID());
            bufferColorRequest.setFilesize(fileLength);
            bufferColorRequest.setFilenameColor(fileNameColor);
            bufferColorRequest.setBufferFinish((( x == target.size() -1) ? "true" : "false"));
            bufferColorRequest.setRequestType(captureSettings.getBufferColorRequestType());
            bufferColorRequest.setDeviceId(GlobalResourceService.getDeviceId());
            bufferColorRequest.setCamera(_deviceConfig.getLayoutPosition());

            String bufferColorRequestStr = gson.toJson(bufferColorRequest);
            _networkService.sendBinaryToHost(bufferColorRequestStr, bufferToSend, true);
            LogService.i(TAG,"send color image to host " + bufferColorRequestStr);
        }
    }

    public void sendAllColorBuffer(B3DCaptureSettings captureSettings) {
        if(colorBufferMap == null || colorBufferMap.isEmpty()) {
            _b3D4ClientError.setErrorCode(IMAGE_BUFFER_INVALID.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
            return;
        }
        ArrayList<String> list= new ArrayList<>(colorBufferMap.keySet());
        Collections.sort(list, new Comparator<String>() {
            @Override
            public int compare(String o1, String o2) {
                return Long.parseLong(o1)>Long.parseLong(o2)?1:-1;
            }
        });
    }

    public void generatePreviewLandMark() {
        if(Arc1PreviewFrame != null) {
            synchronized (Arc1PreviewFrame) {
                generatePreviewLandMarkJNI(Arc1PreviewFrame);
            }
        } else {
            LogService.w(TAG,"previous preview frame not ready yet!!");
        }
    }

    public B3D4ClientError cancelStream(B3DCaptureSettings captureSettings){

        _isCancelStream = true;

        final long start = System.currentTimeMillis();
        while(!( _mDepthCameraState == DepthCameraState.StateType.CONNECTED || // if state is connected, it's free to go
                 _mDepthCameraState == DepthCameraState.StateType.STREAMING    // wait for previous scan request finish open camera and start stream
              ) && System.currentTimeMillis() - start < 3000 /* give 3s max */) {
            try {
                LogService.d(TAG,"wait for depth camera finish start streamd");
                LogService.i(TAG,"current DepthCameraState is " + _mDepthCameraState + "...");
                Thread.sleep(50);
            } catch (InterruptedException e) {
                LogService.e(TAG,e.getMessage());
                e.printStackTrace();
                LogService.logStackTrace(TAG, e.getStackTrace());
            }
        }

        _b3D4ClientError = _depthCamera.stopStreamSync();

        /* do not return error since still need to make sure cache is clear */
        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
        }

        _b3D4ClientError = _depthCamera.closeSync();
        if( _b3D4ClientError.getErrorCode() != _B3D_OK) {
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
        }

        if(!_depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF, "0")) {
            _b3D4ClientError.setErrorCode(PROJECTOR_TURN_OFF_FAIL.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
        }

        if(!_depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_OFF, "0")) {
            _b3D4ClientError.setErrorCode(FLOOD_TURN_OFF_FAIL.getValue());
            reportError(_b3D4ClientError, captureSettings);
            LogService.e(TAG,_b3D4ClientError.getErrorMessage(_b3D4ClientError.getErrorCode()));
        }

        try {
             /* we don't have buffer_release for arc1 and motion capture yet
            *  still need to delete it here
            * */
            removeDirectory(ARC_ONE_MMAP_FOLDER_PATH);
            removeDirectory(ARC_MOTION_MMAP_FOLDER_PATH);

            // snapshot buffers
            if (snapShotBufferMap != null) {
                snapShotBufferMap.clear();
                snapShotBufferMap = null;
            }

            // color buffers
            // don't set colorBufferMap to null to avoid cancel stream cause
            // null pointer exception
            // its ok to have several buffers left in the map
            // because we will create a new colorBufferMap before every request
            if (colorBufferMap != null) {
                colorBufferMap.clear();
            }

            // buffer_capture buffers
            if(mmBufferCapture != null) {
                /*
                 * check if current buffer capture cache is in cache map or not
                 * if not, remove it directly
                 */
                LogService.i(TAG," try to release prev capture buffer " + mmBufferCapture.getFolderName());
                if( bufferCacheMap!= null)
                {
                    String mmFolder = ARC_STILL_MMAP_FOLDER_PATH + "/" + mmBufferCapture.getFolderName();
                    LogService.i(TAG,"start remove " + mmFolder);
                    File deletetarget = new File(mmFolder);
                    try {
                        if(deletetarget.isDirectory())
                            FileUtils.deleteDirectory(deletetarget);
                    } catch (IOException e) {
                        LogService.e(TAG,e.getMessage());
                        e.printStackTrace();
                        LogService.logStackTrace(TAG, e.getStackTrace());
                    }
                } else {
                    LogService.i(TAG,"buffer id " + mmBufferCapture.getFolderName() + " is in bufferCacheMap");
                }
            }
        } catch (Exception e) {
            LogService.e(TAG, e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        }

        return _b3D4ClientError;
    }

    public void checkMMapUsage() {
        mmapCacheLock.lock();
        try {
            if (bufferCacheMap == null || bufferCacheMap.isEmpty()) {
                LogService.i(TAG, "no valid bufferCacheMap to check");
                return;
            }

            Iterator<Map.Entry<String, MMFrameBuffer>> iterator
                    = bufferCacheMap.entrySet().iterator();

            while (iterator.hasNext()) {
                Map.Entry<String, MMFrameBuffer> entry = iterator.next();
                boolean canRelease = entry.getValue().getCanRelease();
                boolean isProcessing = entry.getValue().getisProcessing();

                LogService.d(TAG, "buffer : " + entry.getKey() + ", canRelease : " + canRelease + ", isProcessing : " + isProcessing);
                if (canRelease && !isProcessing) {
                    String mmFolder = ARC_STILL_MMAP_FOLDER_PATH + "/" + entry.getKey();
                    LogService.e(TAG, "start release " + mmFolder);

                    File deletetarget = new File(mmFolder);
                    if (deletetarget.isDirectory())
                        FileUtils.deleteDirectory(deletetarget);
                    iterator.remove();
                    LogService.d(TAG, "current buffer remained " + bufferCacheMap.size());
                }
            }
            LogService.i(TAG, "done check, current buffer remained " + bufferCacheMap.size());
        } catch (IOException e) {
            LogService.e(TAG, e.getMessage());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        } catch (Exception e) {
            LogService.e(TAG, e.toString());
            e.printStackTrace();
            LogService.logStackTrace(TAG, e.getStackTrace());
        } finally {
            mmapCacheLock.unlock();
        }
    }

    private native void generatePreviewLandMarkJNI(byte[] arc1PreviewFrame);

    /* calibration tool, RATestTool use */
    public void start(){
        _depthCamera.registerStreamListener(mDepthCameraStreamListener);
        _depthCamera.openSync();
        _depthCamera.setDecodeTypeJNI(B3DDepthCamera.DecodeType.CALIBRATIONTOOL.ordinal());
        _depthCamera.startStreamSync();
    }

    public void stopStream(){
        _depthCamera.stopStreamSync();
    }

    public void close(){
        _depthCamera.closeSync();
    }

    public void startFlood(){
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
        _depthCamera.setFlood(ctrl,level);
    }

    public void stopFlood(){
        FloodStatus = B3DDepthCamera.Control.CONTROL_OFF;
        _depthCamera.setFlood(B3DDepthCamera.Control.CONTROL_OFF,"0");
    }

    public void startProjector(){
        B3DDepthCamera.Control ctrl = B3DDepthCamera.Control.CONTROL_OFF;
        String level = "0";
        if(ProjectorStatus.equals(B3DDepthCamera.Control.CONTROL_ON)) {
            ProjectorStatus = B3DDepthCamera.Control.CONTROL_OFF;
            ctrl = B3DDepthCamera.Control.CONTROL_OFF;
            level = "0";
        } else {
            ProjectorStatus = B3DDepthCamera.Control.CONTROL_ON;
            ctrl = B3DDepthCamera.Control.CONTROL_ON;
            level = "255";
        }
        _depthCamera.setProject(ctrl,level);
    }

    public void stopProject(){
        ProjectorStatus = B3DDepthCamera.Control.CONTROL_OFF;
        _depthCamera.setProject(B3DDepthCamera.Control.CONTROL_OFF,"0");
    }

    public void setFrameJNI(byte[] byteData, long mTotalFramesStreamed, long currentTimeStamp, int mResolution, boolean isCapture) {
        _depthCamera.SetFrameJNI(byteData,mTotalFramesStreamed,currentTimeStamp,mResolution,isCapture);
    }
}
